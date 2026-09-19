#include "DungeonModel.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"

namespace {
bool SafeSlot(const FString& Slot){
 if(Slot.IsEmpty()||Slot.Len()>100)return false;
 for(TCHAR C:Slot)if(!((C>='a'&&C<='z')||(C>='A'&&C<='Z')||(C>='0'&&C<='9')||C=='_'||C=='-'))return false;
 return true;
}
FString DiskPath(const FString& Slot){return FPaths::ProjectSavedDir()/TEXT("SaveGames")/Slot;}
bool ReadSummary(const FString& File,FJourneySummary& Row){
 FString Raw;TSharedPtr<FJsonObject> O;
 if(!FFileHelper::LoadFileToString(Raw,*File)||!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Raw),O)||!O)return false;
 if(!O->TryGetStringField(TEXT("character"),Row.Character)||!O->TryGetStringField(TEXT("runId"),Row.RunId))return false;
 O->TryGetStringField(TEXT("slot"),Row.Slot);O->TryGetStringField(TEXT("label"),Row.Label);O->TryGetStringField(TEXT("class"),Row.ClassName);
 O->TryGetStringField(TEXT("location"),Row.Location);O->TryGetStringField(TEXT("savedUtc"),Row.UpdatedUtc);
 O->TryGetNumberField(TEXT("level"),Row.Level);O->TryGetBoolField(TEXT("hardcore"),Row.Hardcore);O->TryGetBoolField(TEXT("ended"),Row.Ended);
 return true;
}
void SortJourneys(TArray<FJourneySummary>& Rows){Rows.Sort([](const FJourneySummary& A,const FJourneySummary& B){return A.UpdatedUtc==B.UpdatedUtc?A.Slot>B.Slot:A.UpdatedUtc>B.UpdatedUtc;});}
bool SummaryFromSave(const UDungeonModel& Model,const FString& Character,const FString& Slot,FJourneySummary& Row){
 auto Saved=Cast<UDungeonSave>(UGameplayStatics::LoadGameFromSlot(Model.CharacterSlot(Character,Slot),0));
 if(!Saved||!Saved->CharacterName.Equals(Character,ESearchCase::IgnoreCase)||Saved->Party.IsEmpty()||!Model.Classes.IsValidIndex(Saved->Party[0].Class)||!Model.Floors.IsValidIndex(Saved->Floor))return false;
 Row.Character=Saved->CharacterName;Row.Slot=Slot;Row.Label=Saved->SavedLabel;Row.RunId=Saved->RunId;Row.UpdatedUtc=Saved->SavedUtc;
 Row.ClassName=Model.Classes[Saved->Party[0].Class].Name;Row.Level=Saved->Party[0].Level;Row.Location=Saved->InTown?TEXT("Lonemoore"):Model.Floors[Saved->Floor].Name;Row.Hardcore=Saved->Hardcore;Row.Ended=Saved->Dead;
 return true;
}
}

bool UDungeonModel::ValidateCharacterName(const FString& Name,FString& Error)const{
 Error.Empty();
 if(Name.IsEmpty()){Error=TEXT("Enter a name for your character.");return false;}
 if(Name.Len()>24){Error=TEXT("Choose a name with 24 characters or fewer.");return false;}
 if(Name!=Name.TrimStartAndEnd()){Error=TEXT("A name cannot begin or end with a space.");return false;}
 bool HasLetterOrNumber=false;
 for(TCHAR C:Name){
  const bool AlphaNumeric=FChar::IsAlnum(C);
  if(!AlphaNumeric&&C!=' '&&C!='-'&&C!='\''&&C!='_'){Error=TEXT("Use letters, numbers, spaces, apostrophes, hyphens or underscores.");return false;}
  HasLetterOrNumber|=AlphaNumeric;
 }
 if(!HasLetterOrNumber){Error=TEXT("Include at least one letter or number.");return false;}
 const FString Upper=Name.ToUpper();
 bool Reserved=Upper=="CON"||Upper=="PRN"||Upper=="AUX"||Upper=="NUL";
 if(Upper.Len()==4&&(Upper.StartsWith("COM")||Upper.StartsWith("LPT"))){const TCHAR C=Upper[3];Reserved|=(C>='0'&&C<='9')||C==0x00b9||C==0x00b2||C==0x00b3;}
 if(Reserved){Error=TEXT("That name is reserved by Windows. Please choose another.");return false;}
 return true;
}
FString UDungeonModel::CharacterRoot()const{
 // A unique SavePrefix isolates automated reviews and tests from player files.
 return SavePrefix==TEXT("Lonemoore_")?FString(TEXT("Characters")):SavePrefix+TEXT("Characters");
}
FString UDungeonModel::CharacterDirectory(const FString& Name)const{
 FString Error;if(!ValidateCharacterName(Name,Error)||!SafeSlot(CharacterRoot()))return FString();
 return DiskPath(CharacterRoot()/Name);
}
FString UDungeonModel::CharacterSlot(const FString& Name,const FString& Slot)const{
 if(CharacterDirectory(Name).IsEmpty()||!SafeSlot(Slot))return FString();
 return CharacterRoot()/Name/Slot;
}
bool UDungeonModel::CharacterExists(const FString& Name)const{
 FString Error;if(!ValidateCharacterName(Name,Error))return false;
 TArray<FString> Directories;IFileManager::Get().FindFiles(Directories,*(DiskPath(CharacterRoot())/TEXT("*")),false,true);
 for(const FString& Existing:Directories)if(Existing.Equals(Name,ESearchCase::IgnoreCase)){
  TArray<FString> Contents;IFileManager::Get().FindFiles(Contents,*(DiskPath(CharacterRoot())/Existing/TEXT("*")),true,true);
  return !Contents.IsEmpty(); // An interrupted, empty reservation can be retried safely.
 }
 return false;
}
FString UDungeonModel::DeathLedgerSlot(const FString& Kind,const UDungeonSave* Journey)const{
 if(!Journey)Journey=State;
 if(!Journey||(Kind!="Deaths"&&Kind!="Fallen"))return FString();
 const FString Slot=Kind+"_"+Journey->RunId;
 return Journey->CharacterName.IsEmpty()?Slot:CharacterSlot(Journey->CharacterName,Slot);
}
bool UDungeonModel::WriteJourneyMetadata(const FString& Slot){
 if(!State||State->CharacterName.IsEmpty()||State->Party.IsEmpty())return false;
 const FString Directory=CharacterDirectory(State->CharacterName);if(Directory.IsEmpty())return false;
 const auto& Hero=State->Party[0];if(!Classes.IsValidIndex(Hero.Class))return false;
 auto O=MakeShared<FJsonObject>();O->SetNumberField(TEXT("schema"),1);
 O->SetStringField(TEXT("character"),State->CharacterName);O->SetStringField(TEXT("runId"),State->RunId);
 O->SetStringField(TEXT("slot"),Slot);O->SetStringField(TEXT("label"),State->SavedLabel);
 O->SetStringField(TEXT("class"),Classes[Hero.Class].Name);O->SetNumberField(TEXT("level"),Hero.Level);
 O->SetStringField(TEXT("location"),State->InTown?TEXT("Lonemoore"):Floors[State->Floor].Name);
 O->SetStringField(TEXT("savedUtc"),State->SavedUtc);O->SetBoolField(TEXT("hardcore"),State->Hardcore);O->SetBoolField(TEXT("ended"),State->Dead);
 FString Raw;FJsonSerializer::Serialize(O,TJsonWriterFactory<TCHAR,TPrettyJsonPrintPolicy<TCHAR>>::Create(&Raw));
 auto Write=[&](const FString& File){const FString Temp=File+TEXT(".tmp");return FFileHelper::SaveStringToFile(Raw,*Temp,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)&&IFileManager::Get().Move(*File,*Temp,true,true);};
 if(!Slot.IsEmpty()&&(!SafeSlot(Slot)||!Write(Directory/(Slot+TEXT(".json")))))return false;
 return Write(Directory/TEXT("profile.json"));
}
void UDungeonModel::RefreshCharacters(){
 Characters.Empty();TArray<FString> Directories;
 IFileManager::Get().FindFiles(Directories,*(DiskPath(CharacterRoot())/TEXT("*")),false,true);
 for(const FString& Folder:Directories){
  FString Error;FJourneySummary Row;if(!ValidateCharacterName(Folder,Error))continue;
  if(!ReadSummary(CharacterDirectory(Folder)/TEXT("profile.json"),Row)||!Row.Character.Equals(Folder,ESearchCase::IgnoreCase)){
   TArray<FString> Files;IFileManager::Get().FindFiles(Files,*(CharacterDirectory(Folder)/TEXT("*.sav")),true,false);
   for(const FString& File:Files){const FString Slot=FPaths::GetBaseFilename(File);if(Slot.StartsWith("Deaths_")||Slot.StartsWith("Fallen_")||!SafeSlot(Slot))continue;
    FJourneySummary Recovered;if(SummaryFromSave(*this,Folder,Slot,Recovered)&&(Row.Slot.IsEmpty()||Recovered.UpdatedUtc>Row.UpdatedUtc))Row=Recovered;
   }
   if(Row.Slot.IsEmpty()){
    TArray<FString> Contents;IFileManager::Get().FindFiles(Contents,*(CharacterDirectory(Folder)/TEXT("*")),true,true);if(Contents.IsEmpty())continue;
    Row.Character=Folder;Row.ClassName=TEXT("Save details unavailable");Row.Level=0;Row.Location=TEXT("Select to inspect saved journeys");
   }
  }
  if(Row.Hardcore)Row.Ended|=UGameplayStatics::DoesSaveGameExist(CharacterSlot(Folder,"Fallen_"+Row.RunId),0);
  Characters.Add(Row);
 }
 SortJourneys(Characters);CharacterPage=FMath::Clamp(CharacterPage,0,FMath::Max(0,(Characters.Num()-1)/6));
}
void UDungeonModel::RefreshSaves(const FString& Character){
 BrowseCharacter=Character;SaveSlots.Empty();SavePage=0;
 const FString Directory=CharacterDirectory(Character);if(Directory.IsEmpty())return;
 TArray<FString> Files;IFileManager::Get().FindFiles(Files,*(Directory/TEXT("*.sav")),true,false);
 for(const FString& File:Files){
  const FString Slot=FPaths::GetBaseFilename(File);if(!SafeSlot(Slot)||Slot.StartsWith("Deaths_")||Slot.StartsWith("Fallen_"))continue;
  FJourneySummary Row;
  if(!ReadSummary(Directory/(Slot+TEXT(".json")),Row)){
   // Recover a real save if writing its small sidecar was interrupted.
   if(!SummaryFromSave(*this,Character,Slot,Row))continue;
  }
  if(!Row.Character.Equals(Character,ESearchCase::IgnoreCase)||Row.Slot!=Slot)continue;
  if(Row.Hardcore)Row.Ended|=UGameplayStatics::DoesSaveGameExist(CharacterSlot(Character,"Fallen_"+Row.RunId),0);
  SaveSlots.Add(Row);
 }
 SortJourneys(SaveSlots);
}
bool UDungeonModel::LoadCharacter(const FString& Character,const FString& Slot){
 const FString Path=CharacterSlot(Character,Slot);if(Path.IsEmpty())return false;
 return LoadStoredSlot(Path,Character);
}
bool UDungeonModel::ContinueJourney(){
 RefreshCharacters();
 // Most recently saved character first, then their newest usable slot.
 for(const auto& Character:Characters){if(Character.Ended)continue;RefreshSaves(Character.Character);for(const auto& Save:SaveSlots)if(!Save.Ended&&LoadCharacter(Save.Character,Save.Slot))return true;}
 Say(TEXT("No saved journey is available. Start a new game to create one."));return false;
}

bool UDungeonModel::Save(const FString& Slot){
 if(Combat||Screen=="GameOver"||Screen=="Menu"||Screen=="Select"||Screen=="NameHero"||Screen=="Intro"||!State||State->Party.IsEmpty()||State->Dead||State->Ending==2||!SafeSlot(Slot))return false;
 if(Testing)return true;
 FString ActualSlot=Slot,Path;
 if(!State->CharacterName.IsEmpty()){
  const FString Directory=CharacterDirectory(State->CharacterName);if(Directory.IsEmpty())return false;
  FJourneySummary Owner;
  if(IFileManager::Get().DirectoryExists(*Directory)){
   bool Owned=ReadSummary(Directory/TEXT("profile.json"),Owner);
   if(Owned)Owned=Owner.RunId==State->RunId;
   else{
    TArray<FString> Files;IFileManager::Get().FindFiles(Files,*(Directory/TEXT("*.sav")),true,false);
    for(const FString& File:Files){FJourneySummary Existing;if(SummaryFromSave(*this,State->CharacterName,FPaths::GetBaseFilename(File),Existing)){
     if(Existing.RunId!=State->RunId){Owned=false;break;}Owned=true;
    }}
    if(!Owned){TArray<FString> Contents;IFileManager::Get().FindFiles(Contents,*(Directory/TEXT("*")),true,true);Owned=Contents.IsEmpty();}
   }
   if(!Owned){SaveToast=TEXT("Save failed: this character folder belongs to another journey.");SaveToastUntil=FPlatformTime::Seconds()+5;return false;}
  }else if(!IFileManager::Get().MakeDirectory(*Directory,true))return false;
  if(Slot=="Manual")do{ActualSlot="Manual_"+FDateTime::UtcNow().ToString(TEXT("%Y%m%d_%H%M%S"))+"_"+FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8);}while(UGameplayStatics::DoesSaveGameExist(CharacterSlot(State->CharacterName,ActualSlot),0));
  Path=CharacterSlot(State->CharacterName,ActualSlot);
  State->SavedUtc=FDateTime::UtcNow().ToIso8601();
  State->SavedLabel=Slot=="Manual"?TEXT("Manual save"):Slot=="Auto"?TEXT("Autosave"):Slot=="Quick"?TEXT("Quicksave"):Slot=="BeforeAstra"?TEXT("Before Astra"):Slot;
 }else Path=SavePrefix+Slot; // Explicit unnamed test/review fixtures only.
 const bool OK=UGameplayStatics::SaveGameToSlot(State,Path,0);
 if(OK){
  LastSavedSlot=ActualSlot;SaveToast=Slot=="Auto"?TEXT("Game saved automatically"):TEXT("Game saved");SaveToastUntil=FPlatformTime::Seconds()+3.5;
  if(!State->CharacterName.IsEmpty()&&!WriteJourneyMetadata(ActualSlot)){SaveToast=TEXT("Game saved; its listing could not be updated.");UE_LOG(LogTemp,Warning,TEXT("Profile metadata write failed for %s"),*Path);}
  if(Slot!="Auto")Say("Saved: "+(State->CharacterName.IsEmpty()?Slot:State->CharacterName+" / "+State->SavedLabel));
 }else{SaveToast=TEXT("Save failed. Please try again.");SaveToastUntil=FPlatformTime::Seconds()+5.;}
 return OK;
}
bool UDungeonModel::HasSave(const FString& Slot)const{
 if(!SafeSlot(Slot))return false;
 const FString Path=State&&!State->CharacterName.IsEmpty()?CharacterSlot(State->CharacterName,Slot):SavePrefix+Slot;
 return !Path.IsEmpty()&&UGameplayStatics::DoesSaveGameExist(Path,0);
}
bool UDungeonModel::Load(const FString& Slot){
 if(!SafeSlot(Slot))return false;
 return State&&!State->CharacterName.IsEmpty()?LoadCharacter(State->CharacterName,Slot):LoadStoredSlot(SavePrefix+Slot);
}
