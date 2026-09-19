#include "DungeonModel.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "JsonObjectConverter.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace {
struct FDensityStats {int32 Sites=0,Rooms=0,Ordinary=0,FirstDistance=MAX_int32,RecruitDistance=MAX_int32,MaxRoomSites=0;};
FDensityStats Density(const UDungeonModel* M,int F){
 const auto& R=M->State->Floors[F];const auto& Rows=M->Floors[F].Rows;FDensityStats Out;TMap<FIntPoint,int> Index;int Start=0;
 for(int I=0;I<R.RoomPlacements.Num();++I){const auto& P=R.RoomPlacements[I];Index.Add(FIntPoint(P.X,P.Y),I);if(Rows[P.Y][P.X]=='S')Start=I;}
 TArray<int> Dist;Dist.Init(MAX_int32,R.RoomPlacements.Num());TArray<int> Queue{Start};Dist[Start]=0;const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
 for(int Q=0;Q<Queue.Num();++Q){const auto& P=R.RoomPlacements[Queue[Q]];for(int D=0;D<4;++D)if(P.ActiveSockets&(1<<D))if(auto N=Index.Find(FIntPoint(P.X+DX[D]*7,P.Y+DY[D]*7)))if(Dist[*N]==MAX_int32){Dist[*N]=Dist[Queue[Q]]+1;Queue.Add(*N);}}
 for(int I=0;I<R.RoomPlacements.Num();++I){const auto& P=R.RoomPlacements[I];int Enemies=0;bool StartRoom=false;for(int Y=-3;Y<=3;++Y)for(int X=-3;X<=3;++X){Enemies+=Rows[P.Y+Y][P.X+X]=='E';StartRoom|=Rows[P.Y+Y][P.X+X]=='S';if(Rows[P.Y+Y][P.X+X]=='R')Out.RecruitDistance=FMath::Min(Out.RecruitDistance,Dist[I]);}
  Out.Sites+=Enemies;Out.Rooms+=Enemies>0;Out.MaxRoomSites=FMath::Max(Out.MaxRoomSites,Enemies);if(Enemies)Out.FirstDistance=FMath::Min(Out.FirstDistance,Dist[I]);
  if(!StartRoom&&!P.Id.StartsWith(TEXT("boss_"))&&P.Id!=TEXT("sealed_descent"))++Out.Ordinary;
 }return Out;
}
void Report(const FString& File,const FString& Text){const FString Dir=FPaths::ProjectSavedDir()/TEXT("RoomKitPolish");IFileManager::Get().MakeDirectory(*Dir,true);FFileHelper::SaveStringToFile(Text,*(Dir/File));}
FString RecordText(const FFloorRecord& R){FString Out;FJsonObjectConverter::UStructToJsonObjectString(R,Out);return Out;}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRoomEncounterCoverage,"Dungeon.RoomKit.EncounterRoomCoverage",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRoomEncounterCoverage::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();if(!M->Initialize())return false;M->Testing=true;M->NewGame(0,false);FString Text;int Cases=0;
 for(int F=0;F<M->Floors.Num();++F){int Minimum=MAX_int32,Maximum=0,LatestFirst=0;
  for(int S=0;S<24;++S){M->GenerateModularFloor(F,73519+F*7919+S*104729);const auto Stats=Density(M,F);const auto& R=M->State->Floors[F];FString Error;
   if(!TestTrue(TEXT("Dense floor retains all geometry and gate invariants"),M->ValidateFloor(F,Error))){AddError(Error);return false;}
   if(!TestTrue(TEXT("At least 70 percent of exploration rooms contain encounters"),Stats.Rooms>=FMath::CeilToInt(Stats.Ordinary*.70f)))return false;
   if(!TestTrue(*FString::Printf(TEXT("First regular encounter appears within two room exits (floor=%d seed=%d first_distance=%d)"),F,73519+F*7919+S*104729,Stats.FirstDistance),Stats.FirstDistance<=2))return false;
   if(M->RecruitForFloor(F)>=0&&!TestTrue(TEXT("Companion is reachable within two room exits, before the distant key expedition"),Stats.RecruitDistance<=2))return false;
   if(!TestEqual(TEXT("Fresh encounters occupy distinct rooms, never a global cluster"),Stats.MaxRoomSites,1))return false;
   if(!TestTrue(TEXT("Encounter count scales with the smaller floor instead of crowding 70 sites into it"),Stats.Sites>=45&&Stats.Sites<=48&&Stats.Sites>=R.OriginalEncounters*7))return false;
   TMap<int,int> Copies;for(auto Pair:R.EncounterSources)++Copies.FindOrAdd(Pair.Value);int Low=MAX_int32,High=0;for(auto Pair:Copies){Low=FMath::Min(Low,Pair.Value);High=FMath::Max(High,Pair.Value);}
   if(!TestTrue(TEXT("Existing enemy rosters are balanced and reused at least seven times"),Low>=7&&High==Low))return false;
   Minimum=FMath::Min(Minimum,Stats.Sites);Maximum=FMath::Max(Maximum,Stats.Sites);LatestFirst=FMath::Max(LatestFirst,Stats.FirstDistance);++Cases;
  }
  Text+=FString::Printf(TEXT("floor=%d seeds=24 encounter_sites=%d..%d latest_first_room=%d PASS\n"),F,Minimum,Maximum,LatestFirst);
 }
 Text+=FString::Printf(TEXT("cases=%d; each floor covers >=70%% exploration rooms, no clustered fresh rooms, first encounter <=2 room exits, balanced original enemy rosters.\n"),Cases);Report(TEXT("enemy_distribution_results.txt"),Text);return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRoomEncounterMigration,"Dungeon.RoomKit.EncounterMigrationPreservesProgress",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRoomEncounterMigration::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(0,false);constexpr int F=2;M->State->Floor=F;M->State->InTown=true;
 auto& R=M->State->Floors[F];auto& Rows=M->Floors[F].Rows;const int W=Rows[0].Len();TMap<int,int> Retained;
 TArray<int> Keys;R.EncounterSources.GetKeys(Keys);Keys.Sort();TMap<int,int> Counts;
 for(int C:Keys){int Source=R.EncounterSources[C];if(Counts.FindRef(Source)<10){Retained.Add(C,Source);++Counts.FindOrAdd(Source);}else Rows[C/W][C%W]='.';}
 R.EncounterSources=Retained;R.EncounterRevision=0;R.Layout=Rows;
 int Defeated=Retained.CreateConstIterator().Key();R.Defeated.Add(Defeated,7);
 const auto Marked=R.RoomPlacements[10];for(int Y=-3;Y<=3;++Y)for(int X=-3;X<=3;++X){int C=(Marked.Y+Y)*W+Marked.X+X;R.Seen.AddUnique(C);R.MemoryTiles.Add(C,FString::Chr(Rows[Marked.Y+Y][Marked.X+X]));}
 R.Markers.Add(Marked.Y*W+Marked.X);const auto OldRows=Rows;const auto OldRecord=R;
 if(!TestTrue(TEXT("Old sparse floor gains encounters only in untouched rooms"),M->RepairModularEncounters(F)))return false;
 TestEqual(TEXT("Encounter repair records its completed revision"),R.EncounterRevision,1);TestTrue(TEXT("Defeated enemies are never resurrected"),R.Defeated.OrderIndependentCompareEqual(OldRecord.Defeated));
 TestTrue(TEXT("Discoveries and markers stay unchanged"),R.Seen==OldRecord.Seen&&R.MemoryTiles.OrderIndependentCompareEqual(OldRecord.MemoryTiles)&&R.Markers==OldRecord.Markers);
 TestTrue(TEXT("Door progress and room transforms stay intact"),R.OpenDoors==OldRecord.OpenDoors&&R.UnlockedDoors==OldRecord.UnlockedDoors&&R.RoomPlacements.Num()==OldRecord.RoomPlacements.Num());
 for(int Y=0;Y<Rows.Num();++Y)for(int X=0;X<W;++X)if(Rows[Y][X]!=OldRows[Y][X]){int C=Y*W+X;if(!TestTrue(TEXT("Repair only adds enemies on empty undiscovered floor"),OldRows[Y][X]=='.'&&Rows[Y][X]=='E'&&!OldRecord.Seen.Contains(C)&&!OldRecord.MemoryTiles.Contains(C)))return false;
  const auto* P=R.RoomPlacements.FindByPredicate([&](const FRoomPlacement& Room){return FMath::Abs(Room.X-X)<=3&&FMath::Abs(Room.Y-Y)<=3;});if(!TestNotNull(TEXT("Added encounter remains in an authored room"),P))return false;
  for(int DY=-3;DY<=3;++DY)for(int DX=-3;DX<=3;++DX)if(!TestFalse(TEXT("Entire repaired room was previously unseen"),OldRecord.Seen.Contains((P->Y+DY)*W+P->X+DX)||OldRecord.MemoryTiles.Contains((P->Y+DY)*W+P->X+DX)))return false;
 }
 for(auto Pair:OldRecord.EncounterSources)if(!TestEqual(TEXT("Original live and defeated encounter rosters remain at their saved locations"),R.EncounterSources.FindRef(Pair.Key),Pair.Value))return false;
 const FString Once=RecordText(R);TestFalse(TEXT("Repair does not accumulate on repeated entry or load"),M->RepairModularEncounters(F));TestEqual(TEXT("Repeated repair is field-for-field unchanged"),RecordText(R),Once);
 auto Protected=OldRecord;R=Protected;Rows=R.Layout;M->State->CorpseFloor=F;const auto CorpseRows=Rows;TestFalse(TEXT("Corpse-recovery floor receives no new enemies"),M->RepairModularEncounters(F));TestTrue(TEXT("Corpse route preserved"),Rows==CorpseRows);M->State->CorpseFloor=-1;
 R=Protected;Rows=R.Layout;M->State->Bosses.AddUnique(F);TestFalse(TEXT("Completed floor receives no new enemies"),M->RepairModularEncounters(F));TestTrue(TEXT("Completed floor preserved"),Rows==Protected.Layout);M->State->Bosses.Remove(F);
 M->GenerateExpandedFloor(F,73519);const FString Legacy=RecordText(M->State->Floors[F]);TestFalse(TEXT("Existing version-three encounter behavior remains unchanged"),M->RepairModularEncounters(F));TestEqual(TEXT("Legacy record remains exact"),RecordText(M->State->Floors[F]),Legacy);
 Report(TEXT("enemy_migration_results.txt"),TEXT("PASS: additive old-v4 repair; every modified room unseen; original live/defeated encounter roster locations and progress retained; repeat calls unchanged; corpse/completed/v3 floors unchanged.\n"));return !HasAnyErrors();
}
#endif
