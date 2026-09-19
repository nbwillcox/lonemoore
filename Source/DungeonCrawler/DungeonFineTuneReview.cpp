#include "DungeonGame.h"
#include "DungeonPresentation.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
namespace {
void PreserveExpandedFixture(UDungeonModel* M,int F){const auto& Rows=M->Floors[F].Rows;for(int Y=0;Y<Rows.Num();++Y)for(int X=0;X<Rows[Y].Len();++X)if(Rows[Y][X]=='S')M->State->Floors[F].Seen.AddUnique(Y*Rows[Y].Len()+X);}
}

void ADungeonController::TickFineTuneReview(float Delta){
 static int Stage=0,Failures=0,Floor=-1,EnemyCell=-1;static FString Report;
 ReviewTime+=Delta;if(ReviewTime<4)return;ReviewTime=0;
 auto M=Model.Get();auto S=M->State;
 auto Check=[&](bool OK,const FString& Text){Failures+=!OK;Report+=(OK?"PASS ":"FAIL ")+Text+"\n";UE_LOG(LogTemp,Display,TEXT("FINETUNE_QA %s %s"),OK?TEXT("PASS"):TEXT("FAIL"),*Text);};
 auto Shot=[&](const FString& Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("FineTune")/(Name+".png"),true,false);};
 if(Stage==0){
  M->Testing=true;M->NewGame(0,false);M->Recruit(1);M->Recruit(3);M->TimedCombat=false;
  for(int I=0;I<M->Floors.Num();++I)if(M->Floors[I].RegionIndex==3){Floor=I;break;}
  if(Floor<0){Check(false,"Warrens floor exists");FPlatformMisc::RequestExitWithStatus(false,1);return;}
  M->GenerateExpandedFloor(Floor,73519);PreserveExpandedFixture(M,Floor);M->EnterFloor(Floor);++Stage;return;
 }
 auto& D=M->Floors[Floor];auto& R=S->Floors[Floor];int W=D.Rows[0].Len();
 auto Position=[&](int C,int Facing){S->X=C%W;S->Y=C/W;S->Facing=Facing;M->Reveal();LastX=-1;};
 switch(Stage++){
 case 1:{
  Check(UGameUserSettings::GetGameUserSettings()->IsVSyncEnabled(),"Display synchronization enabled");
  for(const auto& E:R.Boundaries)if(E.Kind=="Wall"&&D.Rows[E.A/W][E.A%W]=='.'){
   int DX=E.B%W-E.A%W,DY=E.B/W-E.A/W;Position(E.A,DX>0?1:DX<0?3:DY>0?2:0);break;
  }break;
 }
 case 2:Shot("01_wall_close");break;
 case 3:S->Facing=(S->Facing+1)%4;LastX=-1;break;
 case 4:Shot("02_wall_grazing");break;
 case 5:{
  bool Found=false;const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
  for(int Y=1;Y<D.Rows.Num()-1&&!Found;++Y)for(int X=1;X<W-1&&!Found;++X)if(D.Rows[Y][X]=='E')for(int Dir=0;Dir<4;++Dir){int NX=X-DX[Dir],NY=Y-DY[Dir];if(D.Rows[NY][NX]=='.'&&M->CanCross(Floor,NY*W+NX,Y*W+X)){EnemyCell=Y*W+X;Position(NY*W+NX,Dir);Found=true;break;}}
  Check(Found,"Visible enemy encounter selected");break;
 }
 case 6:{
  bool Grounded=WorldEnemyVisuals.Num()>0,Scaled=Grounded;
  for(const auto& V:WorldEnemyVisuals)if(auto A=V.Actor.Get()){
   auto Scale=A->GetActorScale3D();const FString Art=A->Tags.Num()>1?A->Tags[1].ToString():FString();Grounded&=FMath::IsNearlyEqual(A->GetActorLocation().Z,DungeonPresentation::GroundedEnemyCenter(Scale.Y*100,SpriteBottomPadding.FindRef(Art)),.1f);
   Scaled&=Scale.X>=2.f;
  }
  Check(Grounded&&Scaled,"World enemy art doubled with feet anchored to ground");Shot("03_world_enemies");break;
 }
 case 7:{
  // ReviewMode supplies a unique prefix; exercise the real save path outside player slots.
  M->Testing=false;M->Save("Manual");M->Testing=true;
  Check(M->HasSave("Manual")&&M->SaveToast=="Game saved"&&M->SaveToastUntil>FPlatformTime::Seconds(),"Manual save writes successfully before success toast");
  Shot("04_save_toast");break;
 }
 case 8:Check(FPlatformTime::Seconds()>M->SaveToastUntil,"Toast expires without needing input");Shot("05_toast_expired");break;
 case 9:M->StartCombat(EnemyCell);M->SaveToast.Empty();Check(!M->Save("Blocked")&&!M->HasSave("Blocked")&&M->SaveToast.IsEmpty(),"Combat save rejected without false success notification");break;
 case 10:Shot("06_combat");break;
 case 11:Check(ArchitectureBuilds==1,"Save, view changes and encounter reuse architecture");M->Combat=false;M->Enemies.Empty();Floor=M->Floors.Num()-1;M->GenerateExpandedFloor(Floor,73519+Floor*7919);PreserveExpandedFixture(M,Floor);M->EnterFloor(Floor);break;
 case 12:{S->Keys.AddUnique(D.Key);S->Bosses.AddUnique(Floor);for(const auto& E:R.Boundaries)if(E.Mandatory)M->OpenBoundary(E);M->TickDoors(1);M->WorldDirty=true;bool Found=false;for(int Y=1;Y<D.Rows.Num()&&!Found;++Y)for(int X=0;X<W&&!Found;++X)if(D.Rows[Y][X]=='A'){Position((Y-3)*W+X,2);Found=true;}Check(Found,"Final boss ceiling review position exists");break;}
 case 13:{bool Fits=WorldEnemyVisuals.Num()>0;for(const auto& V:WorldEnemyVisuals)if(auto A=V.Actor.Get())Fits&=A->GetActorLocation().Z+A->GetActorScale3D().Y*50<=520.1;Check(Fits,"Tall enemies fit below the lowest dungeon ceiling");Shot("07_final_boss_clearance");break;}
 default:
  Check(ArchitectureBuilds==2,"Only changing floors rebuilds the architecture");
  Check(UGameplayStatics::DeleteGameInSlot(M->SavePrefix+"Manual",0),"Temporary review save removed");
  FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("FineTune/results.txt")));
  UE_LOG(LogTemp,Display,TEXT("FINETUNE_REVIEW_COMPLETE failures=%d"),Failures);
  FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);break;
 }
}
