#include "DungeonModel.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonCharacterNames,"Dungeon.Profiles.NamesAndSevenClasses",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonCharacterNames::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();if(!TestTrue(TEXT("Campaign initializes"),M->Initialize()))return false;
 M->SavePrefix="ProfileNames_"+FGuid::NewGuid().ToString(EGuidFormats::Digits)+"_";
 const auto Original=M->State;TestFalse(TEXT("Player campaign requires a name"),M->NewGame(0,false));TestTrue(TEXT("Rejected name leaves state intact"),M->State==Original);
 FString Error;
 for(const FString Name:{FString(),FString(" "),FString("../Escape"),FString("..\\Escape"),FString("NUL"),FString("con"),FString("COM1"),FString("Lpt9"),FString("A:B"),FString("End."),FString("A\nB"),FString("/Absolute"),FString("A*B"),FString(" leading"),FString("trailing "),FString("1234567890123456789012345"),FString("COM")+FString::Chr(0x00b9)}){
  TestFalse(*("Reject unsafe name: "+Name),M->ValidateCharacterName(Name,Error));TestFalse(TEXT("Rejected name has clear guidance"),Error.IsEmpty());
 }
 for(const FString Name:{FString("Elara"),FString("Sir Rowan-2"),FString("O'Neal"),FString("Mage_Seven")})TestTrue(*("Accept safe name: "+Name),M->ValidateCharacterName(Name,Error));
 M->Testing=true;
 for(int32 Class=0;Class<7;++Class){
  const FString Name="Hero "+FString::FromInt(Class);if(!TestTrue(TEXT("Every existing class starts a named journey"),M->NewGame(Class,false,Name)))return false;
  TestEqual(TEXT("Chosen class is protagonist"),M->State->Party[0].Class,Class);TestEqual(TEXT("Explicit name is used in party"),M->HeroName(M->State->Party[0]),Name);
  TestEqual(TEXT("Named save uses schema 3"),M->State->Schema,3);TestTrue(TEXT("Starting weapon is valid"),M->Item(M->State->Party[0].Gear[0].Id)!=nullptr);
  TestTrue(TEXT("Protagonist is recorded as recruited"),M->State->Recruited.Contains(Class));M->Recruit(Class);TestEqual(TEXT("Protagonist cannot be duplicated"),M->State->Party.Num(),1);
  M->State->Recruited.Remove(Class);M->Recruit(Class);TestEqual(TEXT("Party identity protects even a missing recruitment flag"),M->State->Party.Num(),1);M->State->Recruited.Add(Class);
  TArray<int32> Resolved;for(int F=0;F<M->Floors.Num();++F){const int Recruit=M->RecruitForFloor(F);Resolved.Add(Recruit);if(Recruit>=0)M->Recruit(Recruit);}
  TestEqual(TEXT("All starts can recruit a full five-person party"),M->State->Party.Num(),5);
  TSet<int32> Unique;TSet<FString> Names;for(const auto& Hero:M->State->Party){Unique.Add(Hero.Class);Names.Add(Hero.Name);}
  TestEqual(TEXT("All five classes are distinct"),Unique.Num(),5);TestEqual(TEXT("Companion names remain distinct from hero"),Names.Num(),5);
  for(int F=0;F<M->Floors.Num();++F)TestEqual(TEXT("Recruit identity is stable after recruitment"),M->RecruitForFloor(F),Resolved[F]);
  TestFalse(TEXT("Testing creates no character folder"),IFileManager::Get().DirectoryExists(*M->CharacterDirectory(Name)));
 }
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonCharacterSaves,"Dungeon.Profiles.IsolatedSaveBrowserAndHardcore",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonCharacterSaves::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();if(!TestTrue(TEXT("Campaign initializes"),M->Initialize()))return false;
 const FString Token=FGuid::NewGuid().ToString(EGuidFormats::Digits);M->SavePrefix="ProfileDisk_"+Token+"_";
 const FString SaveBase=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("SaveGames"));
 const FString TestRoot=FPaths::ConvertRelativePathToFull(SaveBase/M->CharacterRoot());
 if(!TestTrue(TEXT("Test directory is a new isolated child"),TestRoot.StartsWith(SaveBase+TEXT("/"))&&TestRoot.Contains(Token)&&!IFileManager::Get().DirectoryExists(*TestRoot)))return false;
 ON_SCOPE_EXIT{IFileManager::Get().DeleteDirectory(*TestRoot,false,true);};
 const FString First="Elara",Second="Rowan";
 M->Testing=true;M->NewGame(3,true,First);M->Town();M->Testing=false;
 M->State->Gold=111;TestTrue(TEXT("First character autosave writes"),M->Save("Auto"));
 M->State->Gold=222;TestTrue(TEXT("First character quicksave writes"),M->Save("Quick"));
 M->State->Gold=333;TestTrue(TEXT("First manual save writes"),M->Save("Manual"));const FString ManualOne=M->LastSavedSlot;
 M->State->Gold=444;M->State->Party[0].Level=3;TestTrue(TEXT("Second manual save writes"),M->Save("Manual"));const FString ManualTwo=M->LastSavedSlot;
 TestTrue(TEXT("Manual saves never reuse a slot"),ManualOne!=ManualTwo&&ManualOne.StartsWith("Manual_")&&ManualTwo.StartsWith("Manual_"));
 M->RefreshCharacters();M->RefreshSaves(First);TestEqual(TEXT("One character listed"),M->Characters.Num(),1);TestEqual(TEXT("Auto Quick and two manuals listed"),M->SaveSlots.Num(),4);
 const auto* Latest=M->SaveSlots.FindByPredicate([&](const FJourneySummary& J){return J.Slot==ManualTwo;});
 TestTrue(TEXT("Save metadata contains class level town and timestamp"),Latest&&Latest->ClassName==M->Classes[3].Name&&Latest->Level==3&&Latest->Location=="Lonemoore"&&!Latest->UpdatedUtc.IsEmpty());
 FString Metadata;TestTrue(TEXT("Readable profile metadata exists"),FFileHelper::LoadFileToString(Metadata,*(M->CharacterDirectory(First)/TEXT("profile.json"))));
 TestTrue(TEXT("Readable metadata identifies character"),Metadata.Contains(First)&&Metadata.Contains(TEXT("savedUtc")));
 TArray<uint8> Before;FFileHelper::LoadFileToArray(Before,*(M->CharacterDirectory(First)/TEXT("Quick.sav")));
 auto BeforeRejected=M->State;TestFalse(TEXT("Case-insensitive duplicate name rejected"),M->NewGame(1,false,"elara"));TestTrue(TEXT("Duplicate preserves active journey"),M->State==BeforeRejected);
 TestFalse(TEXT("Traversal character rejected"),M->LoadCharacter("../Escape","Auto"));TestFalse(TEXT("Traversal slot rejected"),M->LoadCharacter(First,"../Quick"));
 M->Testing=true;M->NewGame(6,false,Second);M->Town();M->Testing=false;M->State->Gold=888;
 TestFalse(TEXT("Second character initially has no quicksave"),M->HasSave("Quick"));TestTrue(TEXT("Second autosave writes"),M->Save("Auto"));TestTrue(TEXT("Second quicksave writes"),M->Save("Quick"));TestTrue(TEXT("Second manual writes"),M->Save("Manual"));
 TArray<uint8> After;FFileHelper::LoadFileToArray(After,*(M->CharacterDirectory(First)/TEXT("Quick.sav")));TestTrue(TEXT("Other character saves are byte-for-byte unchanged"),Before==After);
 M->RefreshCharacters();TestEqual(TEXT("Both characters selectable"),M->Characters.Num(),2);
 auto Restored=NewObject<UDungeonModel>();Restored->Initialize();Restored->SavePrefix=M->SavePrefix;
 TestTrue(TEXT("Load first manual from fresh model"),Restored->LoadCharacter(First,ManualOne));TestEqual(TEXT("First manual retained its own progress"),Restored->State->Gold,333);TestEqual(TEXT("First name is restored"),Restored->HeroName(Restored->State->Party[0]),First);
 TestTrue(TEXT("Active character Quick resolves to its own folder"),Restored->Load("Quick"));TestEqual(TEXT("First Quick is independent from Auto/manual"),Restored->State->Gold,222);
 TestTrue(TEXT("Load second character Quick"),Restored->LoadCharacter(Second,"Quick"));TestEqual(TEXT("Second progress is independent"),Restored->State->Gold,888);
 // Remove only sidecars created by this isolated test, then verify recovery from actual saved games.
 IFileManager::Get().Delete(*(M->CharacterDirectory(First)/TEXT("profile.json")),true,false);
 IFileManager::Get().Delete(*(M->CharacterDirectory(First)/(ManualOne+TEXT(".json"))),true,false);
 Restored->RefreshCharacters();TestEqual(TEXT("Missing profile index cannot hide a saved character"),Restored->Characters.Num(),2);
 Restored->RefreshSaves(First);TestEqual(TEXT("Missing slot sidecar cannot hide a real save"),Restored->SaveSlots.Num(),4);
 TestTrue(TEXT("Recovered character loads"),Restored->LoadCharacter(First,ManualOne));TestTrue(TEXT("Saving repairs recoverable profile metadata"),Restored->Save("Auto"));
 TestTrue(TEXT("Profile metadata repaired"),IFileManager::Get().FileExists(*(M->CharacterDirectory(First)/TEXT("profile.json"))));
 Restored->State->Party[0].HP=0;Restored->RecordHardcoreDeaths();
 TestTrue(TEXT("Hardcore deaths ledger lives inside character folder"),UGameplayStatics::DoesSaveGameExist(Restored->DeathLedgerSlot("Deaths"),0));
 TestFalse(TEXT("Older manual cannot resurrect dead Hardcore protagonist"),Restored->LoadCharacter(First,ManualTwo));Restored->Defeat();
 TestTrue(TEXT("Hardcore end ledger lives inside character folder"),UGameplayStatics::DoesSaveGameExist(Restored->DeathLedgerSlot("Fallen"),0));
 TestFalse(TEXT("Ended Hardcore character cannot load an old save"),Restored->LoadCharacter(First,ManualOne));
 TestTrue(TEXT("Another character still loads after Hardcore death"),Restored->LoadCharacter(Second,"Quick"));
 Restored->RefreshCharacters();TestTrue(TEXT("Save browser shows ended character"),Restored->Characters.ContainsByPredicate([&](const FJourneySummary& J){return J.Character==First&&J.Ended;}));
 TestTrue(TEXT("Continue selects a usable character"),Restored->ContinueJourney());TestEqual(TEXT("Continue skips ended Hardcore journey"),Restored->State->CharacterName,Second);
 // A failed new-name reservation cannot replace the active journey or trap retries.
 const FString EmptyName="Retry Hero";IFileManager::Get().MakeDirectory(*M->CharacterDirectory(EmptyName),true);TestFalse(TEXT("Empty interrupted folder does not reserve name"),M->CharacterExists(EmptyName));
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonCharacterCreation,"Dungeon.Profiles.InitialSaveAndCreationRollback",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonCharacterCreation::RunTest(const FString&){
 auto M=NewObject<UDungeonModel>();if(!TestTrue(TEXT("Campaign initializes"),M->Initialize()))return false;
 const FString Token=FGuid::NewGuid().ToString(EGuidFormats::Digits);M->SavePrefix="ProfileCreate_"+Token+"_";
 const FString SaveBase=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("SaveGames"));
 const FString TestRoot=FPaths::ConvertRelativePathToFull(SaveBase/M->CharacterRoot());
 if(!TestTrue(TEXT("Creation test has a new isolated root"),TestRoot.StartsWith(SaveBase+TEXT("/"))&&TestRoot.Contains(Token)&&!IFileManager::Get().DirectoryExists(*TestRoot)))return false;
 ON_SCOPE_EXIT{IFileManager::Get().DeleteDirectory(*TestRoot,false,true);};
 TestTrue(TEXT("Named player campaign starts"),M->NewGame(5,false,TEXT("Opening Hero")));
 TestEqual(TEXT("Current session still sees introduction"),M->Screen,FString(TEXT("Intro")));
 TestTrue(TEXT("Initial real autosave already exists"),M->HasSave(TEXT("Auto")));
 auto Restored=NewObject<UDungeonModel>();Restored->Initialize();Restored->SavePrefix=M->SavePrefix;
 TestTrue(TEXT("Interrupted introduction has a resumable save"),Restored->LoadCharacter(TEXT("Opening Hero"),TEXT("Auto")));
 TestTrue(TEXT("Interrupted introduction resumes safely in town"),Restored->State->InTown&&Restored->Screen==TEXT("Town")&&Restored->State->Party[0].Class==5);
 Restored->State->Gold=987;auto Original=Restored->State;const FString OriginalScreen=Restored->Screen;
 // This deliberately obstructing file is owned by this test, not an existing character directory.
 const FString Obstruction=Restored->CharacterDirectory(TEXT("Blocked Hero"));const FString Sentinel=TEXT("Do not change this existing file");
 TestTrue(TEXT("Create an isolated write obstruction"),FFileHelper::SaveStringToFile(Sentinel,*Obstruction));
 TestFalse(TEXT("Failed character creation is reported"),Restored->NewGame(1,false,TEXT("Blocked Hero")));
 TestTrue(TEXT("Failed creation restores active campaign and screen"),Restored->State==Original&&Restored->State->Gold==987&&Restored->Screen==OriginalScreen);
 FString After;FFileHelper::LoadFileToString(After,*Obstruction);TestEqual(TEXT("Existing obstruction remains byte-equivalent text"),After,Sentinel);
 TestTrue(TEXT("Original named save remains available"),Restored->HasSave(TEXT("Auto")));
 const FString RetryName=TEXT("Retry Hero");IFileManager::Get().MakeDirectory(*Restored->CharacterDirectory(RetryName),true);
 TestTrue(TEXT("Empty interrupted reservation can become a complete new save"),Restored->NewGame(2,false,RetryName)&&Restored->HasSave(TEXT("Auto")));
 return true;
}
#endif
