#include "DungeonGame.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/PointLight.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"

void ADungeonController::TickCampaignExpansionReview(float Delta){
 static int Floor=0,Stage=-1,Failures=0,Sweeps=0,First=0,Last=17;static FString Report;ReviewTime+=Delta;if(ReviewTime<2.2f)return;ReviewTime=0;
 auto M=Model.Get();auto S=M->State;
 auto Check=[&](bool OK,FString Message){Failures+=!OK;Report+=(OK?"PASS ":"FAIL ")+FString::Printf(TEXT("floor=%d "),Floor)+Message+"\n";UE_LOG(LogTemp,Display,TEXT("CAMPAIGN_EXPANSION_QA %s floor=%d %s"),OK?TEXT("PASS"):TEXT("FAIL"),Floor,*Message);};
 if(Stage==-1){FParse::Value(FCommandLine::Get(),TEXT("ReviewFirst="),First);FParse::Value(FCommandLine::Get(),TEXT("ReviewLast="),Last);Floor=FMath::Clamp(First,0,17);Last=FMath::Clamp(Last,Floor,17);M->Testing=true;M->SavePrefix="CampaignExpansionReview_";M->NewGame(0,false);
  // This review deliberately exercises preserved v3 architecture; RoomKitReview covers v4.
  for(int F=0;F<M->Floors.Num();++F){M->GenerateExpandedFloor(F,73519+F*7919);const auto& Rows=M->Floors[F].Rows;for(int Y=0;Y<Rows.Num();++Y)for(int X=0;X<Rows[Y].Len();++X)if(Rows[Y][X]=='S')M->State->Floors[F].Seen.AddUnique(Y*Rows[Y].Len()+X);}
  Stage=0;return;}
 auto& D=M->Floors[Floor];auto& R=S->Floors[Floor];int W=D.Rows[0].Len();
 auto Find=[&](TCHAR T){for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<W;++X)if(D.Rows[Y][X]==T)return Y*W+X;return -1;};
 auto Position=[&](int C,int Facing){S->X=C%W;S->Y=C/W;S->Facing=Facing;M->Reveal();LastX=-1;};
 auto Shot=[&](FString Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("CampaignExpansion")/FString::Printf(TEXT("Floor_%02d_%s.png"),Floor+1,*Name),true,false);};
 switch(Stage++){
 case 0:M->Combat=false;M->Enemies.Empty();S->Keys.Empty();M->EnterFloor(Floor);break;
 case 1:{FString Error;Check(M->ValidateFloor(Floor,Error),"Layout prerequisites "+Error);int Mismatches=0,Count=0;
  for(const auto& E:R.Boundaries){TCHAR A=D.Rows[E.A/W][E.A%W],B=D.Rows[E.B/W][E.B%W];if((A=='~'||B=='~')&&(E.Kind!="Pit"||A==B))continue;FHitResult Hit;bool Block=GetWorld()->SweepSingleByChannel(Hit,FVector(E.A%W*400,E.A/W*400,155),FVector(E.B%W*400,E.B/W*400,155),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeCapsule(30,80));if(Block==M->CanCross(Floor,E.A,E.B))++Mismatches;++Count;}
  Sweeps+=Count;Check(Mismatches==0,FString::Printf(TEXT("Physical boundary sweeps=%d mismatches=%d"),Count,Mismatches));Check(R.MemoryTiles.Num()<R.OriginalArea,"Entrance fog hides expanded floor");Check(PlayerTorch.IsValid()&&PlayerTorchFill.IsValid(),"Player torch and fill present");
  int Stairs=0,Old=0,RegionMeshes=0;bool Materials=true;FString Expected=FString::Printf(TEXT("SM_Stair_%02d"),Floor);
  for(auto Actor:Scenery){TArray<UInstancedStaticMeshComponent*> Cs;Actor->GetComponents(Cs);for(auto C:Cs)if(auto Mesh=C->GetStaticMesh()){if(Mesh->GetName()==Expected){Stairs+=C->GetInstanceCount();for(int I=0;I<C->GetNumMaterials();++I)Materials&=C->GetMaterial(I)!=nullptr;}Old+=Mesh->GetName()=="SM_Stairs";RegionMeshes+=Mesh->GetPathName().Contains("/RegionalIdentity/");}}
  Check(Stairs==1&&Old==0&&Materials,"One themed staircase with authored materials, no old overlapping stair");Check(D.RegionIndex==1||RegionMeshes>=2,"Regional floor and wall geometry retained");Shot("Entrance");auto V=R.Volumes[1];Position(V.Y*W+V.X,1);break;}
 case 2:Shot("Chamber");break;
 case 3:{auto V=R.Volumes[5];Position(V.Y*W+V.X-5,1);break;}
 case 4:Shot("Bridge");break;
 case 5:{int Exit=Find('>');if(Exit<0)Exit=Find('A');Position(Exit,0);M->Interact();Check(S->Floor==Floor&&M->Screen=="Dungeon"&&!M->CanDescend(),"Forced stair position cannot bypass locks");
  S->Keys.AddUnique(D.Key);for(const auto& E:R.Boundaries)if(E.Mandatory&&E.Requirement!="guardian")M->OpenBoundary(E);M->TickDoors(1);
  int Boss=Find('B');if(Boss>=0){Check(!M->CanDescend(),"Key cannot bypass living boss");for(const auto& E:R.Boundaries)if(E.Requirement=="guardian"){M->OpenBoundary(E);Check(!R.OpenDoors.Contains(E.DoorId),"Boss seal remains closed");}R.Defeated.Add(Boss,S->Transitions);S->Bosses.AddUnique(Floor);for(const auto& E:R.Boundaries)if(E.Requirement=="guardian")M->OpenBoundary(E);M->TickDoors(1);}
  Check(M->CanDescend(),"Legal prerequisites open descent");Position(Exit-2*W,2);M->WorldDirty=true;break;}
 case 6:Shot("Stairs");break;
 case 7:{int Exit=Find('>');if(Exit<0)Exit=Find('A');Position(Exit-2*W+1,2);break;}
 case 8:Shot("StairsSide");break;
 case 9:++Floor;Stage=0;if(Floor<=Last)break;Report+=FString::Printf(TEXT("Floors=%d; physical sweeps=%d; failures=%d\n"),Last-First+1,Sweeps,Failures);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("CampaignExpansion/runtime_results.txt")));UE_LOG(LogTemp,Display,TEXT("CAMPAIGN_EXPANSION_REVIEW_COMPLETE floors=%d sweeps=%d failures=%d"),Last-First+1,Sweeps,Failures);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);break;
 }
}
