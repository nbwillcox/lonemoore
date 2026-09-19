#include "DungeonGame.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

void ADungeonController::TickSharedPropReview(float Delta){
 static FString Report;static int Failures=0,Target=-1;static float Low=100000,High=-100000;static FVector KeyOrigin;
 auto M=Model.Get();auto S=M->State;
 auto Check=[&](bool OK,FString Text){Report+=(OK?"PASS ":"FAIL ")+Text+"\n";Failures+=OK?0:1;UE_LOG(LogTemp,Display,TEXT("SHARED_PROP %s %s"),OK?TEXT("PASS"):TEXT("FAIL"),*Text);};
 if(ReviewStep>=2&&ReviewStep<=3&&!HoverKeys.IsEmpty()&&HoverKeys[0].Actor.IsValid()){
  auto P=HoverKeys[0].Actor->GetActorLocation();Low=FMath::Min(Low,float(P.Z));High=FMath::Max(High,float(P.Z));
 }
 ReviewTime+=Delta;if(ReviewTime<3.f)return;ReviewTime=0;
 auto Capture=[&](FString Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("SharedPropUpdate")/(Name+".png"),true,false);};
 auto Face=[&](TCHAR Tile){
  auto& D=M->Floors[S->Floor];int W=D.Rows[0].Len();const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
  for(int Y=1;Y<D.Rows.Num()-1;++Y)for(int X=1;X<W-1;++X)if(D.Rows[Y][X]==Tile)for(int F=0;F<4;++F){int NX=X-DX[F],NY=Y-DY[F],C=Y*W+X,N=NY*W+NX;if(NX<1||NY<1||NX>=W-1||NY>=D.Rows.Num()-1||D.Rows[NY][NX]!='.')continue;if(!M->CanCross(S->Floor,N,C))continue;Target=C;S->X=NX;S->Y=NY;S->Facing=F;M->Reveal();LastX=-1;return true;}return false;
 };
 auto Props=[&](){int Found=0;for(auto A:Scenery){TArray<UInstancedStaticMeshComponent*> Cs;A->GetComponents(Cs);for(auto C:Cs)if(C->GetStaticMesh()&&(C->GetStaticMesh()->GetPathName().Contains("/SharedInteractables/")||C->GetStaticMesh()->GetPathName().Contains("_Lever"))){++Found;Check(C->GetCollisionEnabled()==ECollisionEnabled::NoCollision,"Shared model has no added collision");Check(C->GetNumMaterials()==2,"Wood and iron material slots preserved");}}Check(Found==2,"Both new shared models loaded into the actual dungeon");};
 switch(ReviewStep++){
 case 0:{M->Testing=true;M->NewGame(0,false);M->EnterFloor(0);auto Settings=UGameUserSettings::GetGameUserSettings();Settings->SetFullscreenMode(EWindowMode::Windowed);Settings->SetScreenResolution(FIntPoint(1600,900));Settings->ApplySettings(false);break;}
 case 1:Check(Face('K'),"Existing key has a reachable viewing tile");Check(HoverKeys.Num()>0,"Uncollected key registered for hover");Props();break;
 case 2:Capture("01_key_float_a");break;
 case 3:{Capture("02_key_float_b");Check(High-Low>19&&High-Low<20.1,FString::Printf(TEXT("Key vertical motion %.2f cm; bounded 20 cm travel"),High-Low));if(!HoverKeys.IsEmpty()&&HoverKeys[0].Actor.IsValid()){auto& K=HoverKeys[0];auto P=K.Actor->GetActorLocation();Check(FMath::IsNearlyEqual(P.X,K.Origin.X)&&FMath::IsNearlyEqual(P.Y,K.Origin.Y),"Key stays anchored to its pickup tile");}M->Interact();Check(S->Keys.Contains(M->Floors[S->Floor].Key)&&S->Floors[S->Floor].Taken.Contains(Target),"Animated key uses unchanged pickup and inventory rules");break;}
 case 4:Check(HoverKeys.IsEmpty(),"Collected key disappears and clears hover registry");Check(Face('L'),"Existing lever has reachable viewing tile");break;
 case 5:Capture("03_lever_game");break;
 case 6:M->Interact();M->TickDoors(1);Check(S->Floors[S->Floor].Switches.Contains(Target),"New lever preserves switch interaction");for(const auto& E:S->Floors[S->Floor].Boundaries)if(E.Requirement=="switch")Check(S->Floors[S->Floor].OpenDoors.Contains(E.DoorId),"Lever still opens its existing gate");Check(Face('C'),"Existing loot crate has reachable viewing tile");break;
 case 7:Capture("04_crate_game");break;
 case 8:{int Gold=S->Gold;M->Interact();Check(S->Floors[S->Floor].Taken.Contains(Target)&&S->Gold>Gold,"New crate preserves loot collection");M->EnterFloor(1);break;}
 case 9:{int Count=HoverKeys.Num();RebuildWorld();Check(Count>0&&Count==HoverKeys.Num(),"Floor transition and world rebuild do not duplicate hover keys");Check(M->Testing,"Campaign save writes disabled");Report+=FString::Printf(TEXT("Failures: %d\n"),Failures);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("SharedPropUpdate/runtime_results.txt")));UE_LOG(LogTemp,Display,TEXT("SHARED_PROP_REVIEW_COMPLETE failures=%d"),Failures);FPlatformMisc::RequestExit(false);break;}
 }
}
