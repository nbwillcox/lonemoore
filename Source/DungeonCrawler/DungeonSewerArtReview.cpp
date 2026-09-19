#include "DungeonGame.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"
#include "GameFramework/GameUserSettings.h"

void ADungeonController::StartSewerArtPlaytest(){
 auto M=Model.Get();M->Testing=true;M->SavePrefix="OCS_Unsaved_";M->NewGame(0,false);
 for(int I=0;I<M->Floors.Num();++I)if(M->Floors[I].RegionIndex==1){M->EnterFloor(I);break;}
 if(ReviewMode){auto Settings=UGameUserSettings::GetGameUserSettings();Settings->SetFullscreenMode(EWindowMode::Windowed);Settings->SetScreenResolution(FIntPoint(1600,900));Settings->ApplySettings(false);}
 const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};int W=M->Floors[M->State->Floor].Rows[0].Len(),Best=-1;
 for(int F=0;F<4;++F){int C=M->Cell(M->State->X,M->State->Y),Depth=0;for(int K=0;K<5;++K){int Next=C+DY[F]*W+DX[F];if(!M->CanCross(M->State->Floor,C,Next))break;++Depth;C=Next;}if(Depth>Best){Best=Depth;M->State->Facing=F;}}M->Reveal();
 M->Say("Old City Sewers art test. Campaign saves are disabled for this test run.");LastX=-1;
 UE_LOG(LogTemp,Display,TEXT("OCS_PLAYTEST_STARTED floor=%d campaign_saves=disabled"),M->State->Floor);
}
void ADungeonController::TickSewerArtReview(float Delta){
 ReviewTime+=Delta;if(ReviewTime<5)return;ReviewTime=0;
 static FString Report;static int Failures=0;
 auto M=Model.Get();auto S=M->State;
 auto Check=[&](bool OK,FString Text){Report+=(OK?"PASS ":"FAIL ")+Text+"\n";Failures+=OK?0:1;UE_LOG(LogTemp,Display,TEXT("OCS_RUNTIME %s %s"),OK?TEXT("PASS"):TEXT("FAIL"),*Text);};
 auto Capture=[&](FString Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("SewerArtIntegration")/(Name+".png"),true,false);};
 auto Scenic=[&](){
  auto& D=M->Floors[S->Floor];auto& R=S->Floors[S->Floor];int W=D.Rows[0].Len(),Best=-1;const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
  for(int Y=1;Y<D.Rows.Num()-1;++Y)for(int X=1;X<W-1;++X)if(D.Rows[Y][X]!='#')for(int F=0;F<4;++F){
   int C=Y*W+X,Depth=0;for(int K=0;K<5;++K){int Next=C+DY[F]*W+DX[F];if(!M->CanCross(S->Floor,C,Next))break;++Depth;C=Next;}
   int Score=Depth*5+(R.Rooms.FindRef(Y*W+X)=="Cistern"?2:0);
   if(Score>Best){Best=Score;S->X=X;S->Y=Y;S->Facing=F;}
  }
  M->Reveal();M->WorldDirty=true;LastX=-1;
 };
 auto Validate=[&](){
  auto& D=M->Floors[S->Floor];auto& R=S->Floors[S->Floor];int W=D.Rows[0].Len(),Mismatch=0,Traced=0,PropBatches=0,Decals=0;TSet<FString> Materials;
  for(const auto& E:R.Boundaries){if(E.B<0)continue;FHitResult Hit;bool Blocked=GetWorld()->SweepSingleByChannel(Hit,FVector(E.A%W*400,E.A/W*400,155),FVector(E.B%W*400,E.B/W*400,155),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeCapsule(30,80));if(Blocked==M->CanCross(S->Floor,E.A,E.B))++Mismatch;++Traced;}
  for(auto A:Scenery){
   TArray<UInstancedStaticMeshComponent*> Meshes;A->GetComponents(Meshes);
   for(auto C:Meshes){
    if(C->GetName().StartsWith("OCS_")){++PropBatches;Check(C->GetCollisionEnabled()==ECollisionEnabled::NoCollision,"Decorative batch has no collision: "+C->GetName());}
    for(int I=0;I<C->GetNumMaterials();++I)if(auto Mat=C->GetMaterial(I))if(Mat->GetPathName().Contains("/OldCitySewers/"))Materials.Add(Mat->GetPathName());
   }
   TArray<UDecalComponent*> Marks;A->GetComponents(Marks);Decals+=Marks.Num();
  }
  Check(Mismatch==0,FString::Printf(TEXT("Sewer floor %d: %d capsule boundary sweeps, %d mismatches"),S->Floor,Traced,Mismatch));
  Check(PropBatches==3&&Decals>=4&&Materials.Num()>=6,FString::Printf(TEXT("Sewer floor %d: %d regional materials, %d prop batches, %d decals"),S->Floor,Materials.Num(),PropBatches,Decals));
  Check(M->Testing,"Campaign save writes disabled during review");
 };
 switch(ReviewStep++){
 case 0:StartSewerArtPlaytest();break;
 case 1:Capture("01_sewer_entrance");Validate();break;
 case 2:Scenic();break;
 case 3:Capture("02_sewer_passage");break;
 case 4:M->Screen="Map";MapFloor=S->Floor;MapPan=FVector2D::ZeroVector;break;
 case 5:Capture("03_sewer_map");Check(!M->Move(1),"Map continues to block gameplay movement");break;
 case 6:{for(int I=S->Floor+1;I<M->Floors.Num();++I)if(M->Floors[I].RegionIndex==1){M->EnterFloor(I);break;}Scenic();break;}
 case 7:Capture("04_rat_king_cistern");Validate();break;
 case 8:{int W=M->Floors[S->Floor].Rows[0].Len();for(int Y=0;Y<M->Floors[S->Floor].Rows.Num();++Y){bool Found=false;for(int X=0;X<W;++X)if(M->Floors[S->Floor].Rows[Y][X]=='E'){M->StartCombat(Y*W+X);Found=true;break;}if(Found)break;}break;}
 case 9:Capture("05_sewer_combat");Check(M->Combat,"Existing 2D enemy combat runs over the 3D sewer");break;
 default:Report+=FString::Printf(TEXT("Runtime failures: %d\n"),Failures);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("SewerArtIntegration/runtime_results.txt")));UE_LOG(LogTemp,Display,TEXT("OCS_REVIEW_COMPLETE failures=%d"),Failures);FPlatformMisc::RequestExit(false);break;
 }
}
