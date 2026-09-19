#include "DungeonModel.h"
#include "DungeonNavigation.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonPerformanceFixtures,"Dungeon.Performance.IdenticalSceneFixtures",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonPerformanceFixtures::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(0,false);M->Recruit(1);M->Recruit(3);
 FString Report;
 for(int F:{0,2,16}){
  M->GenerateExpandedFloor(F,73519+F*7919);
  // Preserve the exact v3 benchmark scene when newer unvisited-floor content exists.
  const auto& FixtureRows=M->Floors[F].Rows;for(int Y=0;Y<FixtureRows.Num();++Y)for(int X=0;X<FixtureRows[Y].Len();++X)if(FixtureRows[Y][X]=='S')M->State->Floors[F].Seen.AddUnique(Y*FixtureRows[Y].Len()+X);
  M->EnterFloor(F);auto& R=M->State->Floors[F];auto V=R.Volumes[F==2?5:1];
  M->State->X=V.X-(F==2?5:0);M->State->Y=V.Y;M->State->Facing=1;M->Reveal();
  FString Error;TestTrue(TEXT("Benchmark uses a valid, gated campaign floor"),M->ValidateFloor(F,Error));
  TArray<uint8> Data;TestTrue(TEXT("Benchmark fixture serializes"),UGameplayStatics::SaveGameToMemory(M->State,Data));
  FString Path=FPaths::ProjectSavedDir()/FString::Printf(TEXT("PerformancePass/Fixtures/Floor_%02d.sav"),F);
  TestTrue(TEXT("Benchmark fixture exported outside player save slots"),FFileHelper::SaveArrayToFile(Data,*Path));
  Report+=FString::Printf(TEXT("floor=%d seed=%d cell=%d,%d facing=1\n"),F,R.LayoutSeed,M->State->X,M->State->Y);
  if(F==2){
   // The fully explored case catches radar cost that only appears late in a run.
   const auto& Rows=M->Floors[F].Rows;int W=Rows[0].Len();
   for(int Y=0;Y<Rows.Num();++Y)for(int X=0;X<W;++X){TCHAR T=Rows[Y][X];if(T=='E'||T=='B'||T=='H')T='.';if(T=='?')T='#';R.MemoryTiles.Add(Y*W+X,FString::Chr(T));}
   for(const auto& E:R.Boundaries)R.MemoryEdges.Add(E.Id,E.Kind=="Secret"?"Wall":E.Kind);
   Data.Empty();UGameplayStatics::SaveGameToMemory(M->State,Data);
   TestTrue(TEXT("Fully explored performance fixture exported"),FFileHelper::SaveArrayToFile(Data,*(FPaths::ProjectSavedDir()/TEXT("PerformancePass/Fixtures/Explored.sav"))));
  }
 }
 FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("PerformancePass/Fixtures/manifest.txt")));return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonMapViewport,"Dungeon.Performance.MapViewportCoverage",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonMapViewport::RunTest(const FString&){
 for(float Zoom:{8.f,27.7778f,60.f})for(FVector2D Focus:{FVector2D(1.5,1.5),FVector2D(42.5,50.5),FVector2D(84.5,98.5),FVector2D(-10,125)}){
  FVector2D Size(250,250),Center(1440,175),TL=Center-Size/2;FDungeonMapTransform T{Center-Focus*Zoom,Zoom,85};auto Rect=T.VisibleCells(TL,Size,99);
  TestTrue(TEXT("Viewport indices stay inside map"),Rect.Min.X>=0&&Rect.Min.Y>=0&&Rect.Max.X<=85&&Rect.Max.Y<=99);
  for(int C=0;C<85*99;++C){auto P=T.CellCenter(C);bool Visible=P.X+Zoom/2>=TL.X&&P.X-Zoom/2<=TL.X+Size.X&&P.Y+Zoom/2>=TL.Y&&P.Y-Zoom/2<=TL.Y+Size.Y;
   if(Visible&&!TestTrue(TEXT("Every visible tile and touching boundary is retained"),Rect.Contains(FIntPoint(C%85,C/85))))return false;
  }
  if(Zoom>20)TestTrue(TEXT("Radar work stays bounded regardless of exploration"),Rect.Area()<=196);
 }
 return true;
}
#endif
