#include "DungeonGame.h"
#include "DungeonRegionalArt.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"

void ADungeonController::StartRegionalArtPlaytest(){
 auto M=Model.Get();M->Testing=true;M->SavePrefix="RegionalArt_Unsaved_";M->NewGame(0,false);int Floor=0;FParse::Value(FCommandLine::Get(),TEXT("ArtFloor="),Floor);M->EnterFloor(FMath::Clamp(Floor,0,M->Floors.Num()-1));M->Say("Regional art test. Campaign saving is disabled for this session.");LastX=-1;
 UE_LOG(LogTemp,Display,TEXT("REGIONAL_PLAYTEST_STARTED floor=%d saves=disabled"),M->State->Floor);
}
void ADungeonController::TickRegionalArtReview(float Delta){
 static FString Report;static int Failures=0,Floor=0,Stage=0,Target=-1;static float Low=100000,High=-100000;static int TotalSweeps=0;
 auto M=Model.Get();auto S=M->State;
 auto Check=[&](bool OK,FString Text){Report+=(OK?"PASS ":"FAIL ")+Text+"\n";Failures+=OK?0:1;UE_LOG(LogTemp,Display,TEXT("REGIONAL_QA %s floor=%d %s"),OK?TEXT("PASS"):TEXT("FAIL"),Floor,*Text);};
 if(Stage>=1&&Stage<=5&&!HoverKeys.IsEmpty()&&HoverKeys[0].Actor.IsValid()){float Z=HoverKeys[0].Actor->GetActorLocation().Z;Low=FMath::Min(Low,Z);High=FMath::Max(High,Z);}
 ReviewTime+=Delta;if(ReviewTime<2.5f)return;ReviewTime=0;
 if(ReviewStep++==0){M->Testing=true;M->NewGame(0,false);M->TimedCombat=false;
  // Save-disabled art fixture: let the hero act first so static combat captures
  // cannot be interrupted by accumulated damage across eighteen encounters.
  for(auto& Hero:M->State->Party)Hero.Stats[1]=200;
  auto Settings=UGameUserSettings::GetGameUserSettings();Settings->SetFullscreenMode(EWindowMode::Windowed);Settings->SetScreenResolution(FIntPoint(1600,900));Settings->ApplySettings(false);return;}
 auto Capture=[&](FString Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("RegionalIdentity")/FString::Printf(TEXT("Floor_%02d_%s.png"),Floor+1,*Name),true,false);};
 auto Face=[&](TCHAR Tile){
  auto& D=M->Floors[S->Floor];int W=D.Rows[0].Len();const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
  for(int Y=1;Y<D.Rows.Num()-1;++Y)for(int X=1;X<W-1;++X)if(D.Rows[Y][X]==Tile)for(int F=0;F<4;++F){int NX=X-DX[F],NY=Y-DY[F];if(NX<1||NY<1||NX>=W-1||NY>=D.Rows.Num()-1||D.Rows[NY][NX]!='.')continue;int C=Y*W+X;if(!M->CanCross(S->Floor,NY*W+NX,C))continue;Target=C;S->X=NX;S->Y=NY;S->Facing=F;M->Reveal();LastX=-1;return true;}return false;
 };
 auto Scenic=[&](){auto& D=M->Floors[S->Floor];int W=D.Rows[0].Len(),Best=-1;const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};for(int Y=1;Y<D.Rows.Num()-1;++Y)for(int X=1;X<W-1;++X)if(D.Rows[Y][X]=='.')for(int F=0;F<4;++F){int C=Y*W+X,Score=0;for(int I=0;I<7;++I){int Next=C+DY[F]*W+DX[F];if(!M->CanCross(S->Floor,C,Next))break;++Score;C=Next;}if(Score>Best){Best=Score;S->X=X;S->Y=Y;S->Facing=F;}}M->Reveal();LastX=-1;};
 auto Validate=[&](){auto& D=M->Floors[S->Floor];auto& R=S->Floors[S->Floor];int W=D.Rows[0].Len(),Mismatch=0,Traced=0,Decor=0,Decals=0,IdentityMeshes=0;TSet<FString> Mats;bool Safe=true;
  for(const auto& E:R.Boundaries){if(E.B<0)continue;FHitResult Hit;bool Block=GetWorld()->SweepSingleByChannel(Hit,FVector(E.A%W*400,E.A/W*400,155),FVector(E.B%W*400,E.B/W*400,155),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeCapsule(30,80));if(Block==M->CanCross(S->Floor,E.A,E.B))++Mismatch;++Traced;}TotalSweeps+=Traced;
  for(auto A:Scenery){TArray<UInstancedStaticMeshComponent*> Cs;A->GetComponents(Cs);for(auto C:Cs){if(C->GetName().StartsWith("ID_")){++IdentityMeshes;Safe&=C->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&C->IsVisible();}if(C->GetName().StartsWith("REG_")||C->GetName().StartsWith("OCS_")){++Decor;Safe&=C->GetCollisionEnabled()==ECollisionEnabled::NoCollision;}for(int I=0;I<C->GetNumMaterials();++I)if(auto Mat=C->GetMaterial(I))if(Mat->GetPathName().Contains("/RegionalArt/")||Mat->GetPathName().Contains("/RegionalIdentity/")||Mat->GetPathName().Contains("/OldCitySewers/"))Mats.Add(Mat->GetPathName());}TArray<UDecalComponent*> Ds;A->GetComponents(Ds);Decals+=Ds.Num();}
  Check(Mismatch==0,FString::Printf(TEXT("%d boundary capsule sweeps, %d mismatches"),Traced,Mismatch));Check(Safe&&Decor>=3&&Decals>=4&&Mats.Num()>=6,FString::Printf(TEXT("Non-colliding dressing: %d batches, %d decals, %d materials"),Decor,Decals,Mats.Num()));
  Check(D.RegionIndex==1?IdentityMeshes==0:IdentityMeshes>=5,FString::Printf(TEXT("Visible regional architecture batches: %d; collision-free"),IdentityMeshes));
  bool KeyOK=!HoverKeys.IsEmpty();for(auto& K:HoverKeys)if(K.Actor.IsValid()){auto C=K.Actor->FindComponentByClass<UStaticMeshComponent>();KeyOK&=C&&C->GetStaticMesh()&&C->GetStaticMesh()->GetPathName()==RegionalArt::KeyFor(D.Key)&&C->GetCollisionEnabled()==ECollisionEnabled::NoCollision;}else KeyOK=false;Check(KeyOK,"Correct themed world key loaded without collision");
 };
 switch(Stage++){
 case 0:M->Combat=false;M->Enemies.Empty();M->EnterFloor(Floor);Low=100000;High=-100000;break;
 case 1:Capture("Entrance");Validate();Scenic();break;
 case 2:Capture("Passage");break;
 case 3:Check(Face('K'),"Reachable key inspection view");break;
 case 4:Capture("Key");break;
 case 5:Check(High-Low>19&&High-Low<20.1,FString::Printf(TEXT("Floating key travel %.2f cm"),High-Low));M->Interact();Check(S->Floors[Floor].Taken.Contains(Target)&&S->Keys.Contains(M->Floors[Floor].Key),"Themed key preserves pickup and lock identity");break;
 case 6:Check(HoverKeys.IsEmpty(),"Collected key visual removed");Check(Face('L'),"Reachable themed lever inspection view");break;
 case 7:Capture("Lever");M->Interact();M->TickDoors(1);Check(S->Floors[Floor].Switches.Contains(Target),"Themed lever preserves switch state");for(const auto& E:S->Floors[Floor].Boundaries)if(E.Requirement=="switch")Check(S->Floors[Floor].OpenDoors.Contains(E.DoorId),"Lever gate opens");break;
 case 8:M->Screen="Map";MapFloor=Floor;MapPan=FVector2D::ZeroVector;Check(!M->Move(1),"Map blocks gameplay movement");break;
 case 9:Capture("Map");break;
 case 10:{M->Screen="Dungeon";Scenic();auto& D=M->Floors[Floor];int W=D.Rows[0].Len();bool Found=false;for(int Y=0;Y<D.Rows.Num()&&!Found;++Y)for(int X=0;X<W;++X)if(D.Rows[Y][X]=='E'){M->StartCombat(Y*W+X);Found=true;break;}Check(Found&&M->Combat,"Existing 2D enemy combat starts");break;}
 case 11:Capture("Combat");break;
 case 12:++Floor;Stage=0;if(Floor<M->Floors.Num())break;Check(M->Testing,"Campaign writes disabled");Report+=FString::Printf(TEXT("Floors: %d; total boundary sweeps: %d; failures: %d\n"),Floor,TotalSweeps,Failures);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("RegionalIdentity/runtime_results.txt")));UE_LOG(LogTemp,Display,TEXT("REGIONAL_REVIEW_COMPLETE floors=%d sweeps=%d failures=%d"),Floor,TotalSweeps,Failures);FPlatformMisc::RequestExit(false);break;
 }
}
