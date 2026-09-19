#include "DungeonModel.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCampaignExpandedSeeds,"Dungeon.CampaignExpansion.AllFloorsSeedsAndGates",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCampaignExpandedSeeds::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(0,false);FString Report;int Cases=0,Injections=0;
 for(int F=0;F<M->Floors.Num();++F){TSet<uint32> Layouts;int Smallest=MAX_int32,Largest=0;
  for(int I=0;I<100;++I){int Seed=73519+F*7919+I*104729;M->GenerateExpandedFloor(F,Seed);FString Error;
   if(!M->ValidateFloor(F,Error)){AddError(FString::Printf(TEXT("floor=%d seed=%d: %s"),F,Seed,*Error));return false;}
   auto Rows=M->Floors[F].Rows;int Area=0;FString Flat;for(const auto& Row:Rows){Flat+=Row;for(TCHAR T:Row)Area+=T!='#'&&T!='~';}Layouts.Add(GetTypeHash(Flat));Smallest=FMath::Min(Smallest,Area);Largest=FMath::Max(Largest,Area);
   M->GenerateExpandedFloor(F,Seed);if(!TestTrue(TEXT("Every floor reproduces its seed"),M->Floors[F].Rows==Rows))return false;
   auto& R=M->State->Floors[F];auto Edges=R.Boundaries;
   TArray<FString> Requirements{M->Floors[F].Key};if(!M->Floors[F].Boss.IsEmpty())Requirements.Add("guardian");
   for(const auto& Requirement:Requirements){for(auto& E:R.Boundaries)if(E.Requirement==Requirement){E.Kind="Open";E.Requirement.Empty();}if(!TestFalse(TEXT("Every campaign lock/boss bypass is rejected"),M->ValidateFloor(F,Error)))return false;R.Boundaries=Edges;++Injections;}
   ++Cases;
  }
  TestEqual(TEXT("100 unique layouts for each floor"),Layouts.Num(),100);
  // Count actual enemy identities for the same party/run, not just map markers.
  M->State->Floor=F;int Expanded=0;for(auto Source:M->State->Floors[F].EncounterSources)Expanded+=M->EncounterRoster(Source.Key).Num();
  M->GenerateFloor(F,73519+F*7919+99*104729);int Former=0;int W=M->Floors[F].Rows[0].Len();for(int Y=0;Y<M->Floors[F].Rows.Num();++Y)for(int X=0;X<W;++X)if(M->Floors[F].Rows[Y][X]=='E')Former+=M->EncounterRoster(Y*W+X).Num();
  TestEqual(TEXT("Exactly ten times each floor's existing enemy rosters"),Expanded,Former*10);
  Report+=FString::Printf(TEXT("floor=%d unique=%d area=%d..%d enemies=%d former=%d PASS\n"),F,Layouts.Num(),Smallest,Largest,Expanded,Former);
 }
 Report+=FString::Printf(TEXT("%d generated layouts, %d deliberate bypass injections\n"),Cases,Injections);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("CampaignExpansion/seed_results.txt")));AddInfo(Report);return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCampaignExpandedMigration,"Dungeon.CampaignExpansion.ContinueJourney",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCampaignExpandedMigration::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(0,false);
 for(int F=0;F<3;++F)M->GenerateFloor(F,73519+F*7919);
 auto Original0=M->Floors[0].Rows,Original1=M->Floors[1].Rows,Original2=M->Floors[2].Rows;
 M->State->Floors[0].Seen.Add(60);M->EnterFloor(0);TestTrue(TEXT("Explored floor remains unchanged"),M->Floors[0].Rows==Original0);
 M->State->Gold=917;auto Party=M->State->Party;M->State->Floors[0].Markers.Add(M->Cell(M->State->X,M->State->Y));auto Record0=M->State->Floors[0];
 M->Testing=false;M->SavePrefix="CampaignExpansionQA_"+FGuid::NewGuid().ToString(EGuidFormats::Digits)+"_";TestTrue(TEXT("Legacy journey saved in isolated slot"),M->Save("Journey"));TestTrue(TEXT("Legacy journey loads"),M->Load("Journey"));M->Testing=true;
 TestTrue(TEXT("Loading never regenerates stored floors"),M->Floors[0].Rows==Original0&&M->Floors[1].Rows==Original1);
 TestEqual(TEXT("Gold retained"),M->State->Gold,917);TestEqual(TEXT("Hero name retained"),M->State->Party[0].Name,Party[0].Name);TestEqual(TEXT("Hero HP retained"),M->State->Party[0].HP,Party[0].HP);
 M->EnterFloor(1);FString Error;TestEqual(TEXT("Unvisited floor expands on first entry"),M->State->Floors[1].ContentVersion,4);TestTrue(TEXT("Upgraded floor remains gated"),M->ValidateFloor(1,Error));
 TestTrue(TEXT("Explored geometry and markers survive adjacent upgrade"),M->Floors[0].Rows==Original0&&M->State->Floors[0].Markers==Record0.Markers);
 M->State->CorpseFloor=2;M->State->CorpseCell=100;TestFalse(TEXT("Corpse recovery prevents floor regeneration"),M->UpgradeUnvisitedFloor(2));TestTrue(TEXT("Corpse floor retained"),M->Floors[2].Rows==Original2);
 UGameplayStatics::DeleteGameInSlot(M->SavePrefix+"Journey",0);return true;
}
#endif
