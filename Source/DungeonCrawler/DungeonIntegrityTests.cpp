#include "DungeonModel.h"
#include "DungeonNavigation.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntegritySeeds,"Dungeon.Integrity.SeedsAndDependencies",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FIntegritySeeds::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();if(!M->Initialize())return false;M->Testing=true;M->NewGame(0,false);int Count=0;double Start=FPlatformTime::Seconds();
 for(int F=0;F<M->Floors.Num();++F)for(int Seed=0;Seed<100;++Seed){M->GenerateFloor(F,73519+Seed*104729);FString Error;if(!M->ValidateFloor(F,Error)){AddError(FString::Printf(TEXT("Seed %d %s"),Seed,*Error));return false;}auto Layout=M->Floors[F].Rows;auto Edges=M->State->Floors[F].Boundaries;M->GenerateFloor(F,73519+Seed*104729);if(Layout!=M->Floors[F].Rows||Edges.Num()!=M->State->Floors[F].Boundaries.Num()){AddError(TEXT("Non-reproducible layout"));return false;}++Count;}
 FString Report=FString::Printf(TEXT("Validated %d configurations x 100 seeds = %d floors. Each includes unavailable-required-gate bypass test, reachable key, switch dependency and legal-action fixed point. %.3f seconds.\n"),M->Floors.Num(),Count,FPlatformTime::Seconds()-Start);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("IntegrityUpdate/seed_results.txt")));AddInfo(Report);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntegrityState,"Dungeon.Integrity.DoorsVisibilityPersistence",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FIntegrityState::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(0,false);M->EnterFloor(0);auto& R=M->State->Floors[0];int W=M->Floors[0].Rows[0].Len();
 const auto* Found=R.Boundaries.FindByPredicate([](const FDungeonBoundary& E){return E.Mandatory;});if(!TestNotNull(TEXT("Required boundary"),Found))return false;auto E=*Found;
 auto Face=[&](int A,int B){M->State->X=A%W;M->State->Y=A/W;M->State->Facing=B%W>A%W?1:B%W<A%W?3:B/W>A/W?2:0;M->Reveal();};Face(E.A,E.B);
 TestFalse(TEXT("Missing key blocks movement"),M->CanCross(0,E.A,E.B));M->State->Keys.Add("Wrong Key");M->Interact();TestFalse(TEXT("Wrong key cannot unlock"),R.UnlockedDoors.Contains(E.DoorId));M->State->Keys.Add(M->Floors[0].Key);TestFalse(TEXT("Possession is not an open door"),M->CanCross(0,E.A,E.B));M->Interact();TestTrue(TEXT("Unlock records persistent ID"),R.UnlockedDoors.Contains(E.DoorId));TestFalse(TEXT("Animation still blocks"),M->CanCross(0,E.A,E.B));M->TickDoors(.2f);TestFalse(TEXT("Partial opening blocks"),M->Move(1));M->TickDoors(1);TestTrue(TEXT("Opened boundary traversable"),M->CanCross(0,E.A,E.B));M->State->Keys.Empty();TestTrue(TEXT("Consumed or removed key does not relock"),M->CanCross(0,E.A,E.B));
 TestFalse(TEXT("Two-tile or diagonal move rejected"),M->Move(2));
 auto Wall=R.Boundaries.FindByPredicate([&](const FDungeonBoundary& B){return B.Kind=="Wall"&&B.B>=0;});if(Wall){Face(Wall->A,Wall->B);TestFalse(TEXT("Wall crossing blocked"),M->Move(1));}
 auto Secret=R.Boundaries.FindByPredicate([](const FDungeonBoundary& B){return B.Kind=="Secret";});if(Secret){Face(Secret->A,Secret->B);M->Reveal();TestEqual(TEXT("Undiscovered secret appears as wall"),R.MemoryEdges.FindRef(Secret->Id),FString("Wall"));M->OpenBoundary(*Secret);M->TickDoors(1);TestTrue(TEXT("Secret discovery persists"),R.Secrets.Contains(Secret->DoorId));}
 auto Bars=R.Boundaries.FindByPredicate([](const FDungeonBoundary& B){return B.Kind=="Bars";});if(Bars){TestTrue(TEXT("Bars admit sight"),M->CanCross(0,Bars->A,Bars->B,true));TestFalse(TEXT("Bars reject movement"),M->CanCross(0,Bars->A,Bars->B));R.Switches.Add(1);M->OpenBoundary(*Bars);M->TickDoors(1);TestTrue(TEXT("Switch opens linked gate"),M->CanCross(0,Bars->A,Bars->B));}
 R.Markers.AddUnique(M->Cell(M->State->X,M->State->Y));auto Memories=R.MemoryTiles;auto Doors=R.OpenDoors;auto Layout=R.Layout;
 M->Testing=false;M->SavePrefix="Integrity_"+FGuid::NewGuid().ToString(EGuidFormats::Digits)+"_";TestTrue(TEXT("Save written"),M->Save("Roundtrip"));R.OpenDoors.Empty();R.Markers.Empty();TestTrue(TEXT("Save reloaded"),M->Load("Roundtrip"));TestTrue(TEXT("Opened doors survive reload without key"),M->State->Floors[0].OpenDoors==Doors);TestTrue(TEXT("Layout retained"),M->Floors[0].Rows==Layout);TestTrue(TEXT("Markers retained"),M->State->Floors[0].Markers.Num()>0);TestTrue(TEXT("Exploration retained"),M->State->Floors[0].MemoryTiles.Num()>=Memories.Num());TestTrue(TEXT("Secrets retained"),M->State->Floors[0].Secrets.Num()>0);TestTrue(TEXT("Switches retained"),M->State->Floors[0].Switches.Num()>0);UGameplayStatics::DeleteGameInSlot(M->SavePrefix+"Roundtrip",0);M->Testing=true;
 FDungeonMapTransform T{FVector2D(117,55),37,W};for(int C=0;C<W*19;++C)TestEqual(TEXT("Map/radar coordinate inverse"),T.CellAt(T.CellCenter(C)),C);
 // Unknown touching room must not be visible through an explicit wall even when both cells are floor.
 auto& Edges=M->State->Floors[0].Boundaries;auto Open=Edges.FindByPredicate([](const FDungeonBoundary& B){return B.Kind=="Open";});if(Open){Open->Kind="Wall";int Hidden=Open->B;M->Floors[0].Rows[Hidden/W][Hidden%W]='E';M->State->Floors[0].MemoryTiles.Add(Hidden,"C");Face(Open->A,Hidden);TestFalse(TEXT("Adjacent room behind wall not visible"),M->Visible.Contains(Hidden));TestFalse(TEXT("Radar does not detect enemy through wall"),M->DetectedEnemies.Contains(Hidden));TestEqual(TEXT("Unseen object does not remotely update"),M->State->Floors[0].MemoryTiles.FindRef(Hidden),FString("C"));Open->Kind="Open";M->Reveal();TestTrue(TEXT("Radar detects enemy with range and sight"),M->DetectedEnemies.Contains(Hidden));Open->Kind="Wall";M->Reveal();TestFalse(TEXT("Live enemy tracking removed when sight lost"),M->DetectedEnemies.Contains(Hidden));}
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntegrityLegacy,"Dungeon.Integrity.LegacySaves",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FIntegrityLegacy::RunTest(const FString&){
 // Supported legacy coverage must not depend on which player saves happen to
 // exist on this machine. Exercise actual serialization without creating slots.
 for(int Version:{2,3}){
  auto M=NewObject<UDungeonModel>();if(!M->Initialize())return false;M->Testing=true;M->NewGame(0,false);
  for(int F=0;F<M->Floors.Num();++F){if(Version==2)M->GenerateFloor(F,73519+F*7919);else M->GenerateExpandedFloor(F,73519+F*7919);}
  const auto Before=M->State->Floors;TArray<uint8> Bytes;
  if(!TestTrue(TEXT("Supported legacy fixture serializes in memory"),UGameplayStatics::SaveGameToMemory(M->State,Bytes)))return false;
  auto Restored=Cast<UDungeonSave>(UGameplayStatics::LoadGameFromMemory(Bytes));if(!TestNotNull(TEXT("Supported legacy fixture deserializes"),Restored))return false;
  M->State=Restored;FString Error;
  if(!TestTrue(TEXT("Supported version-two and version-three topologies restore"),M->RestoreTopology(Error))){AddError(Error);return false;}
  for(int F=0;F<Before.Num();++F)if(!TestTrue(TEXT("Supported legacy layout and version are retained"),M->Floors[F].Rows==Before[F].Layout&&M->State->Floors[F].ContentVersion==Version))return false;
 }
 FString Folder=FPaths::ProjectDir()/TEXT("Builds/Windows/DungeonCrawler/Saved/SaveGames");TArray<FString> Files;IFileManager::Get().FindFiles(Files,*(Folder/TEXT("Lonemoore_*.sav")),true,false);int Count=0,Retired=0;
 for(const auto& File:Files){
  const FString Path=Folder/File;TArray<uint8> Bytes;
  if(!TestTrue(TEXT("Existing campaign source is readable"),FFileHelper::LoadFileToArray(Bytes,*Path)))continue;
  auto Save=Cast<UDungeonSave>(UGameplayStatics::LoadGameFromMemory(Bytes));
  if(!TestNotNull(TEXT("Existing campaign source deserializes without writing it"),Save))continue;
  const bool RetiredRoomKit=Save->Floors.ContainsByPredicate([](const FFloorRecord& R){return R.ContentVersion==4&&R.RoomPlacements.Num()==104&&R.Layout.Num()==100&&!R.Layout.IsEmpty()&&R.Layout[0].Len()==72;});
  auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->State=Save;const auto Layouts=Save->Floors;FString Error;const bool Loaded=M->RestoreTopology(Error);
  if(RetiredRoomKit){
   TestFalse(TEXT("Retired 104-section room-kit saves are explicitly rejected by the 68-section release"),Loaded);
   TestTrue(TEXT("Retired layout reports the intended generation contract mismatch"),Error==TEXT("Invalid modular floor dimensions or placement count"));++Retired;
  }else if(!Loaded){AddError(File+": "+Error);}
  else{
   for(int F=0;F<Layouts.Num();++F)if(!Layouts[F].Layout.IsEmpty()){
    const auto& Before=Layouts[F];const auto& After=M->Floors[F].Rows;bool Preserved=Before.Layout.Num()==After.Num();
    for(int Y=0;Y<Before.Layout.Num()&&Preserved;++Y){Preserved=Before.Layout[Y].Len()==After[Y].Len();for(int X=0;X<Before.Layout[Y].Len()&&Preserved;++X)if(Before.Layout[Y][X]!=After[Y][X]){int C=Y*Before.Layout[Y].Len()+X;Preserved=Before.ContentVersion==4&&Before.EncounterRevision==0&&Before.Layout[Y][X]=='.'&&After[Y][X]=='E'&&!Before.Seen.Contains(C)&&!Before.MemoryTiles.Contains(C);}}
    TestTrue(TEXT("Supported saved geometry and discoveries remain unchanged; sparse current v4 may gain only unseen encounters"),Preserved);
   }++Count;
  }
  TArray<uint8> AfterBytes;
  TestTrue(TEXT("Accepted or rejected player save remains byte-for-byte unchanged on disk"),FFileHelper::LoadFileToArray(AfterBytes,*Path)&&AfterBytes==Bytes);
 }
 AddInfo(FString::Printf(TEXT("Supported v2/v3 serialized fixtures passed; %d existing supported saves restored; %d retired 104-section v4 saves rejected. Existing source bytes checked unchanged; no player-slot writes or deletions."),Count,Retired));return !HasAnyErrors();
}
#endif
