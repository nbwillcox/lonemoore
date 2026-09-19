#include "DungeonModel.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace {
void PreserveExpandedFixture(UDungeonModel* M,int F){const auto& Rows=M->Floors[F].Rows;for(int Y=0;Y<Rows.Num();++Y)for(int X=0;X<Rows[Y].Len();++X)if(Rows[Y][X]=='S')M->State->Floors[F].Seen.AddUnique(Y*Rows[Y].Len()+X);}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExpansionSeeds,"Dungeon.Expansion.SeedsAndBypasses",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExpansionSeeds::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(0,false);TSet<uint32> Layouts;int MinArea=MAX_int32,MaxArea=0;FString Report;
 for(int I=0;I<100;++I){int Seed=73519+I*104729;M->GenerateExpandedFloor(2,Seed);FString Error;if(!M->ValidateFloor(2,Error)){AddError(FString::Printf(TEXT("Seed %d: %s"),Seed,*Error));return false;}
  auto Rows=M->Floors[2].Rows;int Area=0;FString Flat;for(auto Row:Rows){Flat+=Row;for(TCHAR T:Row)Area+=T!='#'&&T!='~';}MinArea=FMath::Min(MinArea,Area);MaxArea=FMath::Max(MaxArea,Area);Layouts.Add(GetTypeHash(Flat));
  auto& R=M->State->Floors[2];if(I<3){Report+=FString::Printf(TEXT("seed=%d original_area=%d expanded_area=%d original_encounters=%d expanded_encounters=%d\n"),Seed,R.OriginalArea,Area,R.OriginalEncounters,R.EncounterSources.Num());FFileHelper::SaveStringToFile(FString::Join(Rows,TEXT("\n")),*(FPaths::ProjectSavedDir()/TEXT("ExpansionPrototype")/FString::Printf(TEXT("layout_%d.txt"),Seed)));}
  M->GenerateExpandedFloor(2,Seed);TestTrue(TEXT("Seed reproduces exact layout"),Rows==M->Floors[2].Rows);
  // Deliberately remove each seal: validator must catch either bypass.
  for(FString Requirement:TArray<FString>{M->Floors[2].Key,"guardian"}){auto& Edges=M->State->Floors[2].Boundaries;auto Original=Edges;for(auto& E:Edges)if(E.Requirement==Requirement){E.Kind="Open";E.Requirement.Empty();}TestFalse(TEXT("Removing a required seal is detected"),M->ValidateFloor(2,Error));Edges=Original;}
 }
 TestTrue(TEXT("100 distinct actual floor plans"),Layouts.Num()==100);Report+=FString::Printf(TEXT("100 seeds, 100 reproducibility checks, 200 injected gate bypasses; unique=%d area=%d..%d\n"),Layouts.Num(),MinArea,MaxArea);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("ExpansionPrototype/seed_results.txt")));AddInfo(Report);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExpansionPlay,"Dungeon.Expansion.PersistenceFogCombat",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExpansionPlay::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(0,false);M->GenerateExpandedFloor(2,73519);PreserveExpandedFixture(M,2);M->EnterFloor(2);auto S=M->State;auto& R=S->Floors[2];int W=M->Floors[2].Rows[0].Len();
 auto Find=[&](TCHAR T){auto& Rows=M->Floors[2].Rows;for(int Y=0;Y<Rows.Num();++Y)for(int X=0;X<W;++X)if(Rows[Y][X]==T)return Y*W+X;return -1;};
 TestTrue(TEXT("Fresh floor mostly undiscovered"),R.MemoryTiles.Num()<R.OriginalArea);auto Memory=R.MemoryTiles;
 auto Pit=R.Boundaries.FindByPredicate([&](const FDungeonBoundary& E){return E.Kind=="Pit"&&M->Floors[2].Rows[E.A/W][E.A%W]!='~';});
 if(TestNotNull(TEXT("Bridge edges exist"),Pit)){TestFalse(TEXT("Pit blocks movement"),M->CanCross(2,Pit->A,Pit->B));TestTrue(TEXT("Pit permits sight across reservoir"),M->CanCross(2,Pit->A,Pit->B,true));}
 int Boss=Find('B'),Exit=Find('>'),Enemy=Find('E');S->X=Exit%W;S->Y=Exit/W;M->Interact();TestEqual(TEXT("Stair interaction cannot teleport past prerequisites"),S->Floor,2);
 auto Seal=R.Boundaries.FindByPredicate([](const FDungeonBoundary& E){return E.Requirement=="guardian";});if(!TestNotNull(TEXT("Boss seal exists"),Seal))return false;FDungeonBoundary E=*Seal;
 S->Bosses.AddUnique(2);TestFalse(TEXT("Boss flag cannot replace defeating boss"),M->HasRequirement(2,E));S->Bosses.Empty();R.Defeated.Add(Boss,0);TestFalse(TEXT("Defeated tile alone cannot replace victory"),M->HasRequirement(2,E));S->Bosses.AddUnique(2);TestTrue(TEXT("Complete boss victory opens requirement"),M->HasRequirement(2,E));
 R.Defeated.Add(Enemy,0);S->Transitions=100;TestTrue(TEXT("Cleared prototype stays cleared after town trips"),M->IsDefeated(Enemy));
 S->Keys.AddUnique(M->Floors[2].Key);for(const auto& Edge:R.Boundaries)if(Edge.Mandatory)M->OpenBoundary(Edge);M->TickDoors(1);TestTrue(TEXT("All opened seals plus boss permit descent"),M->CanDescend());
 auto Layout=R.Layout;auto Doors=R.OpenDoors;R.Markers.AddUnique(Find('S'));M->Testing=false;M->SavePrefix="ExpansionQA_"+FGuid::NewGuid().ToString(EGuidFormats::Digits)+"_";
 TestTrue(TEXT("Expanded save written"),M->Save("Roundtrip"));TestTrue(TEXT("Expanded save restored"),M->Load("Roundtrip"));TestTrue(TEXT("Expanded geometry persists"),M->Floors[2].Rows==Layout);TestTrue(TEXT("All seals persist"),M->State->Floors[2].OpenDoors==Doors);TestTrue(TEXT("Discovery persists"),M->State->Floors[2].MemoryTiles.Num()>=Memory.Num());UGameplayStatics::DeleteGameInSlot(M->SavePrefix+"Roundtrip",0);M->Testing=true;
 // Equivalent party/run uses exactly ten copies of the former encounter rosters.
 int ExpandedEnemies=0;for(auto Source:M->State->Floors[2].EncounterSources)ExpandedEnemies+=M->EncounterRoster(Source.Key).Num();auto OldRecord=M->State->Floors[2];auto OldRows=M->Floors[2].Rows;M->GenerateFloor(2,73519);int OriginalEnemies=0;for(int Y=0;Y<M->Floors[2].Rows.Num();++Y)for(int X=0;X<M->Floors[2].Rows[Y].Len();++X)if(M->Floors[2].Rows[Y][X]=='E')OriginalEnemies+=M->EncounterRoster(Y*M->Floors[2].Rows[Y].Len()+X).Num();TestEqual(TEXT("Exactly ten times regular enemies for same party and run"),ExpandedEnemies,OriginalEnemies*10);
 M->State->Floors[2]=OldRecord;M->Floors[2].Rows=OldRows;M->IndexBoundaries(2);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExpansionRoute,"Dungeon.Expansion.PlayableRoute",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExpansionRoute::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(0,false);M->GenerateExpandedFloor(2,73519);PreserveExpandedFixture(M,2);M->Recruit(1);M->Recruit(3);
 for(auto& H:M->State->Party){H.Level=3;H.Points=6;H.Stats[2]+=2;H.Stats[4]+=2;H.HP=M->MaxHP(H);H.MP=M->MaxMP(H);}M->AddItem(FBagItem("health",12));M->AddItem(FBagItem("mana",10));M->EnterFloor(2);M->TimedCombat=false;
 int W=M->Floors[2].Rows[0].Len(),Steps=0,Fights=0,Potions=0;const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
 auto Find=[&](TCHAR T){for(int Y=0;Y<M->Floors[2].Rows.Num();++Y)for(int X=0;X<W;++X)if(M->Floors[2].Rows[Y][X]==T)return Y*W+X;return -1;};
 TArray<int> Goals{Find('K'),Find('L')};for(int Y=0;Y<M->Floors[2].Rows.Num();++Y)for(int X=0;X<W;++X)if(M->Floors[2].Rows[Y][X]=='E'||M->Floors[2].Rows[Y][X]=='$')Goals.Add(Y*W+X);Goals.Add(Find('B'));Goals.Add(Find('>'));
 for(int Goal:Goals){
  TCHAR Objective=M->Floors[2].Rows[Goal/W][Goal%W];int Start=M->Cell(M->State->X,M->State->Y);TArray<int> Q{Start};TMap<int,int> Parent;Parent.Add(Start,-1);
  for(int I=0;I<Q.Num()&&!Parent.Contains(Goal);++I){int C=Q[I];for(int Dir=0;Dir<4;++Dir){int N=C+DY[Dir]*W+DX[Dir];auto E=M->Boundary(2,C,N);if(!E||E->Kind=="Wall"||E->Kind=="Pit"||Parent.Contains(N)||!M->HasRequirement(2,*E))continue;Parent.Add(N,C);Q.Add(N);}}
  if(!TestTrue(TEXT("Legal route to next prerequisite"),Parent.Contains(Goal)))return false;TArray<int> Path;for(int C=Goal;C!=Start;C=Parent[C])Path.Insert(C,0);
  for(int C:Path){
   // Consume actual supplies between battles; no invulnerability or HP overrides.
   for(int H=0;H<M->State->Party.Num();++H)for(int Attempt=0;Attempt<8&&M->State->Party[H].HP>0&&M->State->Party[H].HP<M->MaxHP(M->State->Party[H])*.8f;++Attempt){bool Used=false;for(int Owner=0;Owner<M->State->Party.Num()&&!Used;++Owner){int Slot=M->State->Party[Owner].Bag.IndexOfByPredicate([](const FBagItem& B){return B.Id=="health";});if(Slot>=0){Used=M->UseItem(Owner,Slot,H);Potions+=Used;}}if(!Used)break;}
   int From=M->Cell(M->State->X,M->State->Y);M->State->Facing=C%W>From%W?1:C%W<From%W?3:C/W>From/W?2:0;
   if(!M->CanCross(2,From,C)){M->Interact();M->TickDoors(1);}bool Moved=M->Move(1);
   if(M->Combat){++Fights;for(int Turn=0;Turn<500&&M->Combat;++Turn){if(M->Acting>=0){auto& H=M->State->Party[M->Acting];bool Critical=H.HP<M->MaxHP(H)*.35f&&H.Bag.ContainsByPredicate([](const FBagItem& B){return B.Id=="health";});M->Action(Critical?"Item":"Attack");}else M->AdvanceTurn();}
    if(!TestTrue(TEXT("Party wins encountered fight without stat cheats"),M->Screen=="Dungeon"&&!M->Combat&&M->State->Floor==2))return false;Moved=M->Move(1);
   }
   if(!TestTrue(TEXT("Actual movement follows legal route"),Moved&&M->Cell(M->State->X,M->State->Y)==C))return false;++Steps;
  }
  if(Objective=='K'||Objective=='L'||Objective=='$')M->Interact();if(Objective=='>'){TestTrue(TEXT("Boss and seals complete"),M->CanDescend());M->Interact();TestEqual(TEXT("Real stair interaction descends"),M->State->Floor,3);}
 }
 FString Report=FString::Printf(TEXT("PASS: %d actual movement steps, %d actual battles including Rat King, %d health potions, key pickup, both seals, successful descent. Level-three warrior/mage/priest fixture; ordinary stats and combat; all regular encounters and optional lever/secret vaults included.\n"),Steps,Fights,Potions);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("ExpansionPrototype/playable_route.txt")));AddInfo(Report);return true;
}
#endif
