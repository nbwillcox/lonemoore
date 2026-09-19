#include "DungeonModel.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace {
FString PlacementSignature(const FFloorRecord& R){
 FString Text;
 for(const auto& P:R.RoomPlacements)Text+=FString::Printf(TEXT("%s:%d,%d:%d:%d;"),*P.Id,P.X,P.Y,P.Rotation,P.ActiveSockets);
 return Text;
}
FString RecordSignature(const FFloorRecord& R){FString Text;FJsonObjectConverter::UStructToJsonObjectString(R,Text);return Text;}
int FindTile(const TArray<FString>& Rows,TCHAR Tile){for(int Y=0;Y<Rows.Num();++Y)for(int X=0;X<Rows[Y].Len();++X)if(Rows[Y][X]==Tile)return Y*Rows[Y].Len()+X;return INDEX_NONE;}
void WriteRoomKitReport(const FString& Name,const FString& Text){
 const FString Dir=FPaths::ProjectSavedDir()/TEXT("RoomKit");IFileManager::Get().MakeDirectory(*Dir,true);FFileHelper::SaveStringToFile(Text,*(Dir/Name));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRoomKitSeeds,"Dungeon.RoomKit.AllFloorsSeedsAndGates",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRoomKitSeeds::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();if(!TestTrue(TEXT("Campaign data loads"),M->Initialize()))return false;
 M->Testing=true;M->NewGame(0,false);M->State->RunId="RoomKitRegression73519";
 constexpr int SeedCount=24;int Cases=0,Injections=0;TSet<FString> KitIds;FString Report;
 for(int F=0;F<M->Floors.Num();++F){
  TSet<uint32> Layouts;int MinArea=MAX_int32,MaxArea=0;
  for(int I=0;I<SeedCount;++I){
   const int Seed=73519+F*7919+I*104729;M->State->Floor=F;
   M->GenerateFloor(F,Seed);TMap<int,int> FormerRosters;
   const auto OriginalRows=M->Floors[F].Rows;const int OldW=OriginalRows[0].Len();
   for(int Y=0;Y<OriginalRows.Num();++Y)for(int X=0;X<OldW;++X)if(OriginalRows[Y][X]=='E')FormerRosters.Add(Y*OldW+X,M->EncounterRoster(Y*OldW+X).Num());
   M->GenerateModularFloor(F,Seed);FString Error;
   if(!M->ValidateFloor(F,Error)){AddError(FString::Printf(TEXT("floor=%d seed=%d: %s"),F,Seed,*Error));return false;}
   auto& R=M->State->Floors[F];const auto Rows=M->Floors[F].Rows;
   if(!TestEqual(TEXT("New floors use authored room content version"),R.ContentVersion,4))return false;
   if(!TestTrue(TEXT("Packed waypoints retain their existing 10000-cell contract"),Rows.Num()*Rows[0].Len()<=10000))return false;
   int Area=0,Encounters=0,Chests=0,Traps=0;FString Flat;
   for(const auto& Row:Rows){Flat+=Row;for(TCHAR Tile:Row){Area+=Tile!='#'&&Tile!='~';Encounters+=Tile=='E';Chests+=Tile=='C';Traps+=Tile=='T';}}
   if(!TestEqual(TEXT("The shorter authored floor contains 68 unchanged-scale sections"),R.RoomPlacements.Num(),68))return false;
   if(!TestEqual(TEXT("Ordinary chest count scales to sixteen"),Chests,16)||!TestEqual(TEXT("Trap count scales to four"),Traps,4))return false;
   for(const auto& P:R.RoomPlacements){int Supplies=0;for(int Y=-3;Y<=3;++Y)for(int X=-3;X<=3;++X)Supplies+=Rows[P.Y+Y][P.X+X]=='C'||Rows[P.Y+Y][P.X+X]=='T';if(!TestTrue(TEXT("Supplies are distributed over distinct rooms"),Supplies<=1))return false;}
   if(!TestTrue(TEXT("Authored walkable area scales the former minimum by 65 percent"),Area*2>=R.OriginalArea*13)||!TestTrue(TEXT("Authored encounters keep the scaled sevenfold roster minimum"),Encounters>=R.OriginalEncounters*7))return false;
   TMap<int,int> SourceCopies;int ActualEnemies=0,OriginalEnemies=0;
   for(const auto& Pair:FormerRosters)OriginalEnemies+=Pair.Value;
   for(const auto& Pair:R.EncounterSources){
    if(!TestTrue(TEXT("Every enemy site points to an original floor roster"),FormerRosters.Contains(Pair.Value)))return false;
    ++SourceCopies.FindOrAdd(Pair.Value);ActualEnemies+=M->EncounterRoster(Pair.Key).Num();
   }
   for(const auto& Pair:FormerRosters)if(!TestTrue(TEXT("Each original roster is reused at least seven times on the shorter floor"),SourceCopies.FindRef(Pair.Key)>=7))return false;
   if(!TestTrue(TEXT("Actual enemy identities retain the scaled sevenfold minimum"),ActualEnemies>=OriginalEnemies*7))return false;
   for(const auto& P:R.RoomPlacements)KitIds.Add(P.Id);
   const FString Modules=PlacementSignature(R);M->GenerateModularFloor(F,Seed);
   if(!TestTrue(TEXT("A seed reproduces both tiles and authored room placement"),M->Floors[F].Rows==Rows&&PlacementSignature(M->State->Floors[F])==Modules))return false;
   Layouts.Add(GetTypeHash(Flat));MinArea=FMath::Min(MinArea,Area);MaxArea=FMath::Max(MaxArea,Area);
   // Removing a real prerequisite must make validation reject the floor, even if its room geometry is otherwise unchanged.
   auto& Generated=M->State->Floors[F];const auto Edges=Generated.Boundaries;
   TArray<FString> Requirements{M->Floors[F].Key};if(!M->Floors[F].Boss.IsEmpty())Requirements.Add("guardian");
   for(const auto& Requirement:Requirements){
    for(auto& E:Generated.Boundaries)if(E.Requirement==Requirement){E.Kind="Open";E.Requirement.Empty();}
    const bool Rejected=!M->ValidateFloor(F,Error);Generated.Boundaries=Edges;
    if(!TestTrue(TEXT("Key and living-boss bypasses are rejected"),Rejected))return false;++Injections;
   }
   ++Cases;
  }
  if(!TestEqual(TEXT("Different seeds produce distinct floor plans"),Layouts.Num(),SeedCount))return false;
  Report+=FString::Printf(TEXT("floor=%d seeds=%d unique=%d area=%d..%d PASS\n"),F,SeedCount,Layouts.Num(),MinArea,MaxArea);
 }
 TestEqual(TEXT("The campaign uses all 25 authored room templates"),KitIds.Num(),25);
 Report+=FString::Printf(TEXT("cases=%d authored_types=%d bypass_injections=%d\n"),Cases,KitIds.Num(),Injections);WriteRoomKitReport(TEXT("seed_results.txt"),Report);AddInfo(Report);return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRoomKitContract,"Dungeon.RoomKit.RejectMalformedAssembly",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRoomKitContract::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(0,false);M->GenerateModularFloor(0,73519);
 auto Original=M->State->Floors[0];FString Error;
 if(!TestTrue(TEXT("Authored module placements exist"),Original.RoomPlacements.Num()>1))return false;
 auto Reject=[&](const TCHAR* Label){const bool Bad=!M->ValidateFloor(0,Error);M->State->Floors[0]=Original;M->IndexBoundaries(0);return TestTrue(Label,Bad);};
 M->State->Floors[0].RoomPlacements[0].Id="missing_room_template";if(!Reject(TEXT("Unknown authored room cannot load")))return false;
 M->State->Floors[0].RoomPlacements[0].X+=1;if(!Reject(TEXT("Misaligned room cannot load")))return false;
 M->State->Floors[0].RoomPlacements[0].Rotation=4;if(!Reject(TEXT("Noncanonical socket rotation cannot load")))return false;
 M->State->Floors[0].RoomPlacements.Add(Original.RoomPlacements[0]);if(!Reject(TEXT("Overlapping duplicate shell cannot load")))return false;
 M->State->Floors[0].RoomPlacements[0].ActiveSockets=16;if(!Reject(TEXT("Invalid portal bits cannot load")))return false;
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRoomKitSaveCompatibility,"Dungeon.RoomKit.SaveAndLegacyJourney",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRoomKitSaveCompatibility::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(0,false);
 for(int F=0;F<6;++F)M->GenerateExpandedFloor(F,73519+F*7919);
 M->GenerateFloor(6,73519+6*7919);
 M->State->InTown=true;M->Screen="Town";M->State->Floor=0;const int Start=FindTile(M->Floors[0].Rows,'S'),W=M->Floors[0].Rows[0].Len();M->State->X=Start%W;M->State->Y=Start/W;
 M->State->Gold=917;M->State->Floors[0].Seen.Add(Start);M->State->Floors[0].Markers.Add(Start);
 M->State->CorpseFloor=2;M->State->CorpseCell=FindTile(M->Floors[2].Rows,'S');M->State->Corpse.Add(FBagItem("health",2));
 M->State->Bosses.AddUnique(3);M->State->Floors[4].Taken.Add(FindTile(M->Floors[4].Rows,'K'));
 const auto Before=M->State->Floors;const auto Party=M->State->Party;
 TArray<FString> LayoutSignatures;for(const auto& R:Before)LayoutSignatures.Add(PlacementSignature(R));
 // Exercise the real disk serialization and Load validation with a unique disposable prefix.
 M->SavePrefix="RoomKitQA_"+FGuid::NewGuid().ToString(EGuidFormats::Digits)+"_";const FString Slot=M->SavePrefix+"Journey";
 ON_SCOPE_EXIT{UGameplayStatics::DeleteGameInSlot(Slot,0);};
 M->Testing=false;if(!TestTrue(TEXT("Mixed legacy and modular journey saves"),M->Save("Journey"))||!TestTrue(TEXT("Mixed journey loads through production path"),M->Load("Journey")))return false;M->Testing=true;
 TestEqual(TEXT("Player gold retained"),M->State->Gold,917);TestEqual(TEXT("Hero name retained"),M->State->Party[0].Name,Party[0].Name);TestEqual(TEXT("Hero HP retained"),M->State->Party[0].HP,Party[0].HP);
 TestEqual(TEXT("Corpse recovery position retained"),M->State->CorpseCell,FindTile(M->Floors[2].Rows,'S'));
 for(int F=0;F<M->Floors.Num();++F){if(!TestTrue(TEXT("Load preserves every existing floor's tiles and authored transforms"),M->Floors[F].Rows==Before[F].Layout&&PlacementSignature(M->State->Floors[F])==LayoutSignatures[F]))return false;}
 for(int F:{0,2,3,4}){const FString Record=RecordSignature(M->State->Floors[F]);TestFalse(TEXT("Exploration, corpse, completed boss and progress each prevent regeneration"),M->UpgradeUnvisitedFloor(F));TestEqual(TEXT("Protected legacy records stay byte-equivalent at field level"),RecordSignature(M->State->Floors[F]),Record);}
 M->State->InTown=false;M->State->Floor=1;const FString Current=RecordSignature(M->State->Floors[1]);TestFalse(TEXT("Occupied legacy floor never regenerates"),M->UpgradeUnvisitedFloor(1));TestEqual(TEXT("Current legacy geometry retained"),RecordSignature(M->State->Floors[1]),Current);M->State->InTown=true;M->State->Floor=0;
 for(int F:{5,6}){FString Error;TestTrue(TEXT("Unvisited legacy floor upgrades"),M->UpgradeUnvisitedFloor(F));TestEqual(TEXT("Unvisited floor gains authored rooms"),M->State->Floors[F].ContentVersion,4);TestTrue(TEXT("Upgraded floor remains playable and gated"),M->ValidateFloor(F,Error));}
 TestTrue(TEXT("Explored v3 layout and markers survive adjacent upgrades"),M->Floors[0].Rows==Before[0].Layout&&M->State->Floors[0].Markers==Before[0].Markers);
 WriteRoomKitReport(TEXT("save_results.txt"),TEXT("Real isolated save/load; module transforms; mixed v2/v3/v4 layouts; occupied/explored/corpse/completed/progress protection; untouched floor upgrade checked.\n"));
 return !HasAnyErrors();
}
#endif
