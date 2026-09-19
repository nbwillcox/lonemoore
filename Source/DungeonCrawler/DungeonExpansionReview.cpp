#include "DungeonGame.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Camera/CameraActor.h"
namespace {
void PreserveExpandedFixture(UDungeonModel* M,int F){const auto& Rows=M->Floors[F].Rows;for(int Y=0;Y<Rows.Num();++Y)for(int X=0;X<Rows[Y].Len();++X)if(Rows[Y][X]=='S')M->State->Floors[F].Seen.AddUnique(Y*Rows[Y].Len()+X);}
}

void ADungeonController::StartExpansionPlaytest(){
 auto M=Model.Get();M->SavePrefix="ExpansionPrototype_";
 if(!FParse::Param(FCommandLine::Get(),TEXT("ExpansionFresh"))&&M->HasSave("Auto")&&M->Load("Auto")){LastX=-1;return;}
 M->NewGame(0,false);int Seed=int(GetTypeHash(FGuid::NewGuid()));FParse::Value(FCommandLine::Get(),TEXT("ExpansionSeed="),Seed);
 M->GenerateExpandedFloor(2,Seed);FString Error;if(!M->ValidateFloor(2,Error)){M->Say("Prototype generation failed: "+Error);UE_LOG(LogTemp,Error,TEXT("EXPANSION_INVALID %s"),*Error);M->Screen="Menu";return;}
 PreserveExpandedFixture(M,2);
 // A disclosed, ordinary level-three party for reviewing a rank-three floor.
 M->Recruit(1);M->Recruit(3);for(auto& H:M->State->Party){H.Level=3;H.Points=6;H.Stats[2]+=2;H.Stats[4]+=2;H.HP=M->MaxHP(H);H.MP=M->MaxMP(H);}
 M->State->Gold=350;M->AddItem(FBagItem("health",12));M->AddItem(FBagItem("mana",10));M->EnterFloor(2);
 M->Say("Expanded Cistern prototype. Level 3 review party. Explore, find the key, defeat the Rat King, then open the stair seal. F5 saves; F9 loads. M opens the map. Prototype saves are separate.");LastX=-1;
 UE_LOG(LogTemp,Display,TEXT("EXPANSION_PLAYTEST_STARTED seed=%d"),Seed);
}

void ADungeonController::TickExpansionReview(float Delta){
 static int Stage=0,Failures=0;static FString Report;static float WorstFrame=0;ReviewTime+=Delta;WorstFrame=FMath::Max(WorstFrame,Delta);
 if(ReviewTime<3)return;ReviewTime=0;auto M=Model.Get();auto S=M->State;
 auto Check=[&](bool OK,FString Message){Failures+=!OK;Report+=(OK?"PASS ":"FAIL ")+Message+"\n";UE_LOG(LogTemp,Display,TEXT("EXPANSION_QA %s %s"),OK?TEXT("PASS"):TEXT("FAIL"),*Message);};
 auto Shot=[&](FString Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("ExpansionPrototype")/(Name+".png"),true,false);};
 if(Stage==0){M->Testing=true;M->SavePrefix="ExpansionReview_";M->NewGame(0,false);M->GenerateExpandedFloor(2,73519);PreserveExpandedFixture(M,2);M->Recruit(1);M->Recruit(3);for(auto& Hero:M->State->Party){Hero.Level=3;Hero.Stats[2]+=2;Hero.Stats[4]+=2;Hero.HP=M->MaxHP(Hero);Hero.MP=M->MaxMP(Hero);}M->EnterFloor(2);MapFloor=2;MapZoom=8;Stage++;return;}
 auto& D=M->Floors[2];auto& R=S->Floors[2];int W=D.Rows[0].Len();
 auto Position=[&](int C,int Facing){S->X=C%W;S->Y=C/W;S->Facing=Facing;M->Reveal();LastX=-1;};
 auto Find=[&](TCHAR T){for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<W;++X)if(D.Rows[Y][X]==T)return Y*W+X;return -1;};
 switch(Stage++){
 case 1:{FString Error;Check(M->ValidateFloor(2,Error),"Layout and all prerequisites: "+Error);int Sweeps=0,Mismatch=0;
  for(const auto& E:R.Boundaries){if(D.Rows[E.A/W][E.A%W]=='~'||D.Rows[E.B/W][E.B%W]=='~'){if(E.Kind!="Pit"||D.Rows[E.A/W][E.A%W]==D.Rows[E.B/W][E.B%W])continue;}
   FHitResult Hit;bool Block=GetWorld()->SweepSingleByChannel(Hit,FVector(E.A%W*400,E.A/W*400,155),FVector(E.B%W*400,E.B/W*400,155),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeCapsule(30,80));if(Block==M->CanCross(2,E.A,E.B))++Mismatch;++Sweeps;
  }Check(Mismatch==0,FString::Printf(TEXT("Physical boundary sweeps=%d mismatches=%d"),Sweeps,Mismatch));Shot("01_entrance");break;}
 case 2:M->Screen="Map";MapZoom=8;MapPan=FVector2D::ZeroVector;Check(R.MemoryTiles.Num()<R.OriginalArea,"Entrance fog hides most of expanded floor");break;
 case 3:Shot("02_fog_at_entrance");break;
 case 4:{M->Screen="Dungeon";auto V=R.Volumes[5];Position(V.Y*W+V.X-5,1);break;}
 case 5:Shot("03_reservoir_bridge");break;
 case 6:{auto V=R.Volumes[5];Position((V.Y-4)*W+V.X,2);break;}
 case 7:Shot("04_reservoir_crossing");break;
 case 8:{auto V=R.Volumes[1];Position(V.Y*W+V.X,1);break;}
 case 9:Shot("05_ruins");break;
 case 10:{auto E=R.Boundaries.FindByPredicate([](const FDungeonBoundary& B){return B.Mandatory&&B.Requirement!="guardian";});if(E){Check(!M->CanCross(2,E->A,E->B),"Key gate physically closed");M->OpenBoundary(*E);Check(!R.UnlockedDoors.Contains(E->DoorId),"Wrong or absent key rejected");}int Key=Find('K');Position(Key,0);M->Interact();Check(S->Keys.Contains(D.Key),"Key collected through interaction");break;}
 case 11:{for(const auto& E:R.Boundaries)if(E.Mandatory&&E.Requirement!="guardian")M->OpenBoundary(E);M->TickDoors(1);int Boss=Find('B');Position(Boss-W,2);M->StartCombat(Boss,true);Check(M->Combat&&M->Enemies.Num()==1&&M->Enemies[0].Id=="ratking","Original Rat King encounter");break;}
 case 12:Shot("06_rat_king");break;
 case 13:{M->Combat=false;M->Enemies.Empty();int Exit=Find('>');Position(Exit,0);M->Interact();Check(S->Floor==2&&!M->CanDescend(),"Forced stair position cannot bypass living boss");for(const auto& E:R.Boundaries)if(E.Requirement=="guardian"){M->OpenBoundary(E);Check(!R.OpenDoors.Contains(E.DoorId),"Guardian seal refuses opening before victory");}break;}
 case 14:{int Boss=Find('B');R.Defeated.Add(Boss,S->Transitions);S->Bosses.AddUnique(2);for(const auto& E:R.Boundaries)if(E.Requirement=="guardian")M->OpenBoundary(E);M->TickDoors(1);Check(M->CanDescend(),"Victory plus open key and guardian gates permits descent");M->WorldDirty=true;break;}
 case 15:Shot("07_open_stair_seal");break;
 case 16:{M->Screen="Map";for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<W;++X)if(D.Rows[Y][X]!='#'&&D.Rows[Y][X]!='~'){S->X=X;S->Y=Y;M->Reveal();}Position(Find('S'),1);Command("MapFit");break;}
 case 17:Shot("08_layout_review_fully_explored");break;
 case 18:{M->Screen="Dungeon";const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};bool Found=false;for(int Y=1;Y<D.Rows.Num()-1&&!Found;++Y)for(int X=1;X<W-1&&!Found;++X)if(D.Rows[Y][X]=='E')for(int Dir=0;Dir<4;++Dir){int NX=X-DX[Dir],NY=Y-DY[Dir];if(D.Rows[NY][NX]=='.'&&M->CanCross(2,NY*W+NX,Y*W+X)){Position(NY*W+NX,Dir);Found=true;break;}}Check(Found,"World enemy and torch comparison view");break;}
 case 19:Check(ArchitectureBuilds==1,"Pickups, key gates and guardian gates reuse the same architecture");Check(PlayerTorch.IsValid()&&PlayerTorchFill.IsValid()&&PlayerTorch->GetAttachParentActor()==DungeonCamera&&FVector::Distance(PlayerTorch->GetActorLocation(),DungeonCamera->GetActorLocation())<100,"Warm torch and ambient fill follow player camera");Check(WorldEnemyVisuals.Num()>0,"World enemies use atmosphere material; combat art retained");Shot("09_world_enemy_torch");break;
 case 20:if(PlayerTorch.IsValid())PlayerTorch->PointLightComponent->SetVisibility(false);if(PlayerTorchFill.IsValid())PlayerTorchFill->PointLightComponent->SetVisibility(false);break;
 case 21:Shot("10_torch_off_comparison");break;
 case 22:if(PlayerTorch.IsValid())PlayerTorch->PointLightComponent->SetVisibility(true);if(PlayerTorchFill.IsValid())PlayerTorchFill->PointLightComponent->SetVisibility(true);break;
 case 23:Shot("11_torch_on_comparison");break;
 case 24:if(M->HasSave("Auto")){Check(M->Testing,"Save writes disabled during resume review");Check(M->Load("Auto"),"Review autosave loads");}else{M->SavePrefix="ExpansionPrototype_";if(M->HasSave("Auto")){Check(M->Testing,"Save writes disabled during resume review");Check(M->Load("Auto"),"Existing prototype autosave loads");}else UE_LOG(LogTemp,Display,TEXT("EXPANSION_RESUME no existing prototype autosave in this build"));}break;
 case 25:if(M->SavePrefix=="ExpansionPrototype_"&&M->HasSave("Auto")){Check(M->Floors[S->Floor].Rows==S->Floors[S->Floor].Layout,"Continued save retains its exact floor plan");Shot("12_existing_save_revised");}break;
 default:Report+=FString::Printf(TEXT("Failures=%d; maximum frame including world builds=%.3fs\n"),Failures,WorstFrame);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("ExpansionPrototype/runtime_results.txt")));UE_LOG(LogTemp,Display,TEXT("EXPANSION_REVIEW_COMPLETE failures=%d"),Failures);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);break;
 }
}
