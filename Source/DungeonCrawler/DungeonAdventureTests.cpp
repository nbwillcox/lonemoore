#include "DungeonModel.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace {
int FindAdventureTile(const TArray<FString>& Rows,TCHAR Tile){for(int Y=0;Y<Rows.Num();++Y)for(int X=0;X<Rows[Y].Len();++X)if(Rows[Y][X]==Tile)return Y*Rows[Y].Len()+X;return -1;}
bool SameLoot(const FBagItem& A,const FBagItem& B){return A.Id==B.Id&&A.Count==B.Count&&A.Quality==B.Quality;}
void AdventureReport(const FString& File,const FString& Text){const FString Dir=FPaths::ProjectSavedDir()/TEXT("AdventurePolish");IFileManager::Get().MakeDirectory(*Dir,true);FFileHelper::SaveStringToFile(Text,*(Dir/File));}
FString Category(const FItemDef& D){if(D.Slot=="Weapon")return "weapons";if(D.Slot.StartsWith(TEXT("Accessory")))return "accessories";if(!D.Slot.IsEmpty())return "armor";if(D.Id=="food")return "food";if(D.Family=="treasure")return "trade relics";return "potions";}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDescentCheckpoint,"Dungeon.Adventure.DescentCheckpointAndGates",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDescentCheckpoint::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();if(!M->Initialize())return false;M->Testing=true;M->NewGame(0,false);FString Report;
 for(int F=0;F<M->Floors.Num();++F){
  M->State->Floor=F;M->State->InTown=false;M->Screen="Dungeon";M->State->Keys.Empty();M->State->Bosses.Empty();M->DoorOpening.Empty();
  auto& R=M->State->Floors[F];R.OpenDoors.Empty();R.UnlockedDoors.Empty();R.Shrines.Empty();R.Defeated.Empty();
  const auto* Room=R.RoomPlacements.FindByPredicate([](const FRoomPlacement& P){return P.Id=="sealed_descent";});
  if(!TestNotNull(TEXT("Each floor has a protected descent chamber"),Room))return false;
  const int W=M->Floors[F].Rows[0].Len(),C=(Room->Y-1)*W+Room->X-1;
  if(!TestTrue(TEXT("Save shrine sits inside protected exit cells"),M->Floors[F].Rows[C/W][C%W]=='V'&&R.ExitCells.Contains(C)&&R.ProtectedCells.Contains(C)))return false;
  FString Error;if(!TestTrue(TEXT("Topology proves the shrine is unreachable before required gates"),M->ValidateFloor(F,Error))){AddError(Error);return false;}
  // Even an invalid external position cannot activate this waypoint early.
  M->State->X=C%W;M->State->Y=C/W;M->Interact();
  if(!TestFalse(TEXT("A sealed checkpoint cannot be registered before the key"),R.Shrines.Contains(C)))return false;
  M->State->Keys.Add(M->Floors[F].Key);
  for(const auto& E:R.Boundaries)if(E.Requirement==M->Floors[F].Key)M->OpenBoundary(E);M->TickDoors(1.f);
  if(!M->Floors[F].Boss.IsEmpty()){
   M->Interact();if(!TestFalse(TEXT("The key alone cannot activate a living guardian's checkpoint"),R.Shrines.Contains(C)))return false;
   const int Boss=FindAdventureTile(M->Floors[F].Rows,'B');R.Defeated.Add(Boss,M->State->Transitions);M->State->Bosses.Add(F);
   for(const auto& E:R.Boundaries)if(E.Requirement=="guardian")M->OpenBoundary(E);M->TickDoors(1.f);
  }
  M->Interact();
  if(!TestTrue(TEXT("Legal guardian completion enables a usable saved waypoint"),R.Shrines.Contains(C)&&M->State->SelectedWaypoint==F*10000+C&&M->Notice.Contains(TEXT("saved"))))return false;
  M->State->InTown=true;M->Screen="Town";M->Waypoint(F*10000+C);
  if(!TestTrue(TEXT("Town return arrives at the descent shrine without changing floor"),!M->State->InTown&&M->State->Floor==F&&M->Cell(M->State->X,M->State->Y)==C))return false;
  Report+=FString::Printf(TEXT("floor=%d checkpoint=%d key/guardian enforced; activation and town return PASS\n"),F,C);
 }
 AdventureReport(TEXT("descent_checkpoints.txt"),Report);return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChestCatalogDepth,"Dungeon.Adventure.ChestCatalogAndDepthQuality",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FChestCatalogDepth::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();if(!M->Initialize())return false;M->Testing=true;M->NewGame(0,false);
 TSet<FString> AllItems,Categories;TArray<double> AverageQuality;FString Report;constexpr int Samples=4096;
 for(int F:{0,M->Floors.Num()/2,M->Floors.Num()-1}){
  auto& R=M->State->Floors[F];const int OldSeed=R.LayoutSeed,C=FindAdventureTile(M->Floors[F].Rows,'C');int QualitySum=0;TSet<int> Qualities;TSet<FString> FloorItems;
  for(int I=0;I<Samples;++I){R.LayoutSeed=73519+I*104729;
   const FBagItem Reward=M->ChestLoot(F,C);const auto* D=M->Item(Reward.Id);
   if(!TestNotNull(TEXT("Every ordinary reward is a real catalog item"),D))return false;
   if(!TestTrue(TEXT("Chest stack and quality remain valid"),Reward.Count>=1&&Reward.Count<=D->Stack&&Reward.Quality>=0&&Reward.Quality<=4))return false;
   AllItems.Add(Reward.Id);FloorItems.Add(Reward.Id);Categories.Add(Category(*D));Qualities.Add(Reward.Quality);QualitySum+=Reward.Quality;
   const FBagItem Sanctuary=M->ChestLoot(F,C,true);
   if(!TestTrue(TEXT("Sanctuary rewards remain premium equipment"),!M->Item(Sanctuary.Id)->Slot.IsEmpty()&&Sanctuary.Quality>=2))return false;
  }
  R.LayoutSeed=OldSeed;const double Mean=double(QualitySum)/Samples;AverageQuality.Add(Mean);
  if(!TestEqual(TEXT("Every current catalog item can appear at each sampled depth"),FloorItems.Num(),M->Items.Num()))return false;
  if(!TestEqual(TEXT("Depth weighting keeps all five qualities possible"),Qualities.Num(),5))return false;
  Report+=FString::Printf(TEXT("floor=%d samples=%d distinct_items=%d mean_quality=%.3f PASS\n"),F,Samples,FloorItems.Num(),Mean);
 }
 if(!TestTrue(TEXT("Loot quality rises substantially with depth without making every item legendary"),AverageQuality[0]<AverageQuality[1]&&AverageQuality[1]<AverageQuality[2]&&AverageQuality[2]>AverageQuality[0]+1.5&&AverageQuality[2]<3.8))return false;
 TestEqual(TEXT("Ordinary chests include weapons, armor, accessories, food, potions and trade relics"),Categories.Num(),6);
 TestEqual(TEXT("The complete current item catalog participates"),AllItems.Num(),M->Items.Num());
 AdventureReport(TEXT("chest_catalog_quality.txt"),Report);return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChestRetryPersistence,"Dungeon.Adventure.ChestFullBagsAndDeterminism",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FChestRetryPersistence::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();if(!M->Initialize())return false;M->Testing=true;M->NewGame(0,false);
 M->State->Floor=0;M->State->InTown=false;M->Screen="Dungeon";const int W=M->Floors[0].Rows[0].Len(),C=FindAdventureTile(M->Floors[0].Rows,'C');M->State->X=C%W;M->State->Y=C/W;
 const FBagItem Expected=M->ChestLoot(0,C);const int Gold=M->State->Gold;
 for(auto& H:M->State->Party)H.Bag.Init(FBagItem("sword",1,1),20);
 for(int Attempt=0;Attempt<3;++Attempt){M->Roll(100);M->Interact();
  if(!TestTrue(TEXT("Full bags preserve the sealed chest and its gold"),!M->State->Floors[0].Taken.Contains(C)&&M->State->Gold==Gold&&M->State->Party[0].Bag.Num()==20))return false;
  if(!TestTrue(TEXT("Failed attempts and other random actions cannot reroll contents"),SameLoot(Expected,M->ChestLoot(0,C))))return false;
 }
 TArray<uint8> Data;if(!TestTrue(TEXT("Unopened chest state serializes"),UGameplayStatics::SaveGameToMemory(M->State,Data)))return false;
 auto Restored=Cast<UDungeonSave>(UGameplayStatics::LoadGameFromMemory(Data));if(!TestNotNull(TEXT("Chest retry save restores"),Restored))return false;M->State=Restored;
 if(!TestTrue(TEXT("Save/load keeps the exact unclaimed contents"),SameLoot(Expected,M->ChestLoot(0,C))&&!M->State->Floors[0].Taken.Contains(C)))return false;
 M->State->Party[0].Bag.Empty();M->Interact();
 if(!TestTrue(TEXT("Making room collects exactly the promised item stack"),M->State->Party[0].Bag.Num()==1&&SameLoot(Expected,M->State->Party[0].Bag[0])&&M->State->Floors[0].Taken.Contains(C)))return false;
 const int ClaimedGold=Gold+15+M->Floors[0].Rank*6;
 if(!TestEqual(TEXT("Gold is granted once when the reward fits"),M->State->Gold,ClaimedGold))return false;
 M->Interact();TestTrue(TEXT("An emptied chest cannot duplicate loot or gold"),M->State->Gold==ClaimedGold&&M->State->Party[0].Bag.Num()==1&&SameLoot(Expected,M->State->Party[0].Bag[0]));
 AdventureReport(TEXT("chest_retry.txt"),TEXT("PASS: full bags preserve reward/gold; global random actions and save/load retain contents; later claim transfers exact stack and gold once; empty chest cannot duplicate.\n"));return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecruitInteractions,"Dungeon.Adventure.RecruitMarkersAndDescentFallback",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRecruitInteractions::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();if(!M->Initialize())return false;M->Testing=true;int Cases=0;FString Report;
 const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
 for(int StartingClass=0;StartingClass<M->Classes.Num();++StartingClass){
  if(!M->NewGame(StartingClass,false))return false;
  const auto Fresh=DuplicateObject<UDungeonSave>(M->State,M);
  for(int Mode=0;Mode<3;++Mode){
   M->State=DuplicateObject<UDungeonSave>(Fresh,M);M->Combat=false;M->DoorOpening.Empty();
   for(int F=0;F<M->Floors.Num();++F){
    const int Companion=M->RecruitForFloor(F);if(Companion<0)continue;
    M->State->Floor=F;M->State->InTown=false;M->Screen="Dungeon";
    auto& R=M->State->Floors[F];const auto& Rows=M->Floors[F].Rows;const int W=Rows[0].Len(),Before=M->State->Party.Num();
    const int Marker=FindAdventureTile(Rows,'R');if(!TestTrue(TEXT("Authored recruit marker exists for every selected companion"),Marker>=0))return false;
    if(Mode==0){
     bool Approached=false;for(int D=0;D<4;++D){const int X=Marker%W+DX[D],Y=Marker/W+DY[D],From=Y*W+X;
      if(X<0||X>=W||Y<0||Y>=Rows.Num()||!M->CanCross(F,From,Marker))continue;
      M->State->X=X;M->State->Y=Y;M->State->Facing=(D+2)%4;Approached=M->Move(1);break;
     }
     if(!TestTrue(TEXT("Walking onto the marker recruits through production movement"),Approached))return false;
    }else if(Mode==1){M->State->X=Marker%W;M->State->Y=Marker/W;M->Interact();}
    else{
     M->State->Keys.AddUnique(M->Floors[F].Key);const int Boss=FindAdventureTile(Rows,'B');
     if(Boss>=0){R.Defeated.Add(Boss,M->State->Transitions);M->State->Bosses.AddUnique(F);}
     for(const auto& Edge:R.Boundaries)if(Edge.Mandatory)M->OpenBoundary(Edge);M->TickDoors(1.f);
     const int Exit=FindAdventureTile(Rows,'>');if(!TestTrue(TEXT("Recruit floor has a legal descent"),Exit>=0&&M->CanDescend()))return false;
     M->State->X=Exit%W;M->State->Y=Exit/W;M->Interact();
     if(!TestEqual(TEXT("Missed-companion fallback preserves normal floor progression"),M->State->Floor,F+1))return false;
    }
    if(!TestTrue(TEXT("Each real recruitment path adds the selected distinct companion"),M->State->Party.Num()==Before+1&&M->State->Recruited.Contains(Companion)))return false;
    if(Mode<2&&Companion!=M->Floors[F].Recruit&&!TestTrue(TEXT("Substitute dialogue names the actual joining class"),M->Notice.StartsWith(M->Classes[Companion].Name+TEXT(":"))))return false;
    if(Mode<2&&!TestTrue(TEXT("Recruit marker becomes consumed"),R.Taken.Contains(Marker)))return false;
    ++Cases;
   }
   TSet<int> Classes;for(const auto& Hero:M->State->Party)Classes.Add(Hero.Class);
   if(!TestEqual(TEXT("Every starting class can form five unique heroes through each recruitment path"),Classes.Num(),5))return false;
   Report+=FString::Printf(TEXT("starter=%d mode=%d unique_party=%d PASS\n"),StartingClass,Mode,Classes.Num());
  }
 }
 Report+=FString::Printf(TEXT("recruitment_cases=%d; movement, inspection and missed-recruit descent tested for all seven starters.\n"),Cases);
 AdventureReport(TEXT("recruit_interactions.txt"),Report);return !HasAnyErrors();
}
#endif
