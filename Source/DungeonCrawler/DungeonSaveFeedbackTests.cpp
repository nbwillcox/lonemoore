#include "DungeonModel.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonSaveFeedback,"Dungeon.UI.SaveConfirmation",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonSaveFeedback::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();TestTrue(TEXT("Campaign initializes"),M->Initialize());
 M->Testing=true;M->SavePrefix="SaveFeedbackTest_"+FGuid::NewGuid().ToString(EGuidFormats::Digits)+"_";
 M->NewGame(0,false);M->Screen="Town";
 TestTrue(TEXT("Dry-run save supported"),M->Save("DryRun"));
 TestTrue(TEXT("Dry-run never claims a disk save"),M->SaveToast.IsEmpty()&&!M->HasSave("DryRun"));
 M->Testing=false;
 for(const FString Slot:{FString("Manual"),FString("Quick"),FString("Auto")}){
  const double Before=FPlatformTime::Seconds();
  TestTrue(TEXT("Real save succeeds"),M->Save(Slot));
  TestTrue(TEXT("Success notification follows actual saved file"),M->HasSave(Slot)&&M->SaveToast.StartsWith("Game saved"));
  TestTrue(TEXT("Toast has a bounded lifetime"),M->SaveToastUntil>=Before+3.5&&M->SaveToastUntil<=FPlatformTime::Seconds()+3.5);
  TestTrue(TEXT("Saved state can be loaded"),UGameplayStatics::LoadGameFromSlot(M->SavePrefix+Slot,0)!=nullptr);
  TestTrue(TEXT("Only temporary test slot removed"),UGameplayStatics::DeleteGameInSlot(M->SavePrefix+Slot,0));
 }
 M->SaveToast.Empty();M->Combat=true;
 TestFalse(TEXT("Combat cannot save"),M->Save("Blocked"));
 TestTrue(TEXT("Blocked save cannot show success or write a slot"),M->SaveToast.IsEmpty()&&!M->HasSave("Blocked"));
 return true;
}
#endif
