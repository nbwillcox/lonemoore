#include "DungeonGame.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/AudioComponent.h"

void ADungeonController::Command(const FString& Id){
 if(WorldLoading)return;
 auto M=Model.Get();auto S=M->State.Get();TArray<FString> P;Id.ParseIntoArray(P,TEXT(":"));int32 A=P.Num()>1?FCString::Atoi(*P[1]):0,B=P.Num()>2?FCString::Atoi(*P[2]):0;
 if(!Id.StartsWith("Action:")&&!Id.StartsWith("Approved:Action:"))PlayCue("ui");
 if(Id=="Cancel"){PendingCommand.Empty();return;}
 if(Id=="Confirm"){FString Next=PendingCommand;PendingCommand.Empty();if(!Next.IsEmpty())Command("Approved:"+Next);return;}
 bool Approved=P[0]=="Approved";if(Approved){P.RemoveAt(0);A=P.Num()>1?FCString::Atoi(*P[1]):0;B=P.Num()>2?FCString::Atoi(*P[2]):0;}
 if(!PendingCommand.IsEmpty())return;
 if(P[0]=="ShopSelect"&&M->Screen=="Service"){Interface->ShopSelection=FMath::Clamp(A,0,4);return;}
 if(M->Screen=="Map"){
  if(Id=="MapFit"){int F=MapFloor>=0?MapFloor:S->Floor;auto& D=M->Floors[F];MapZoom=FMath::Min(1080.f/D.Rows[0].Len(),600.f/D.Rows.Num());FVector2D Focus=F==S->Floor?FVector2D(S->X+.5f,S->Y+.5f):FVector2D(D.Rows[0].Len()/2.f,D.Rows.Num()/2.f);MapPan=(Focus-FVector2D(D.Rows[0].Len()/2.f,D.Rows.Num()/2.f))*MapZoom;return;}
  if(Id=="MapCenter"){MapPan=FVector2D::ZeroVector;return;}
  if(P[0]=="MapZoom"){MapZoom=FMath::Clamp(MapZoom*(A>0?1.25f:.8f),4.f,80.f);return;}
  if(P[0]=="RadarRange"){M->RadarRange=FMath::Clamp(M->RadarRange+A,2,6);M->UpdateDetection();SavePreferences();return;}
  if(P[0]=="MapFloor"){int F=MapFloor>=0?MapFloor:S->Floor;for(int N=F+A;S->Floors.IsValidIndex(N);N+=A)if(!S->Floors[N].Seen.IsEmpty()){MapFloor=N;MapPan=FVector2D::ZeroVector;break;}return;}
 }
 if(P[0]=="Sell"&&!M->SaleItem(A,B))return;
 if((P[0]=="Buy"||P[0]=="Sell"||(P[0]=="Care"&&P[1]=="gamble"))&&!Approved){PendingCommand=Id;ConfirmText=P[0]=="Hero"?"Begin as "+M->Classes[A].Name+(M->HardcoreChoice?" in Hardcore? Death is permanent.":"? Class choice is permanent."):P[0]=="Buy"?"Buy "+M->Item(P[1])->Name+FString::Printf(TEXT(" for %d gold?"),M->Item(P[1])->Value):P[0]=="Sell"?"Sell one "+M->Item(M->SaleItem(A,B)->Id)->Name+FString::Printf(TEXT(" for %d gold?%s"),M->SellPrice(*M->SaleItem(A,B)),B>=100?TEXT(" This removes equipped gear and reduces this hero's stats."):TEXT("")):"Buy a sealed random equipment item?";return;}
 if(P[0]=="Sell"){M->Sell(A,B);return;}
 if(Id=="SellMode"){SellMode=!SellMode;UIPage=0;return;}
 if(Id=="History"){ShowHistory=!ShowHistory;HistoryPage=0;return;}
 if(P[0]=="HistoryPage"){HistoryPage=FMath::Clamp(HistoryPage+A,0,FMath::Max(0,(M->Log.Num()-1)/(M->Screen=="Service"?5:12)));return;}
 if(Id=="CollectLoot"){M->CollectLoot();return;}
 if(P[0]=="SettingsPage"){SettingsPage=A;PageTurn=.3f;return;}
 if(P[0]=="Bind"){BindingCapture=P[1];return;}
 if(Id=="ResetBindings"){Bindings.Empty();const FString Actions[]={"Forward","Backward","Left","Right","Interact","Town","Map","Inventory","Character","Journal","QuickSave","QuickLoad"};const FKey Keys[]={EKeys::W,EKeys::S,EKeys::A,EKeys::D,EKeys::E,EKeys::T,EKeys::M,EKeys::I,EKeys::C,EKeys::J,EKeys::F5,EKeys::F9};for(int32 I=0;I<12;++I)Bindings.Add(Actions[I],Keys[I]);SavePreferences();return;}
 if(P[0]=="Volume"){float& V=P[1]=="Master"?MasterVolume:P[1]=="Music"?MusicVolume:EffectsVolume;V=FMath::Clamp(V+B*.1f,0.f,1.f);SavePreferences();return;}
 if(P[0]=="Brightness"){Brightness=FMath::Clamp(Brightness+A*.1f,.5f,2.f);SavePreferences();M->WorldDirty=true;return;}
 if(P[0]=="GraphicsOption"){auto G=UGameUserSettings::GetGameUserSettings();FString K=P[1];if(K=="Shadows")G->SetShadowQuality(FMath::Clamp(G->GetShadowQuality()+B,0,3));if(K=="Textures")G->SetTextureQuality(FMath::Clamp(G->GetTextureQuality()+B,0,3));if(K=="View distance")G->SetViewDistanceQuality(FMath::Clamp(G->GetViewDistanceQuality()+B,0,3));if(K=="Anti-aliasing")G->SetAntiAliasingQuality(FMath::Clamp(G->GetAntiAliasingQuality()+B,0,3));if(K=="Effects")G->SetVisualEffectQuality(FMath::Clamp(G->GetVisualEffectQuality()+B,0,3));if(K=="Post processing")G->SetPostProcessingQuality(FMath::Clamp(G->GetPostProcessingQuality()+B,0,3));G->ApplySettings(false);return;}
 if(Id=="VSync"){auto G=UGameUserSettings::GetGameUserSettings();G->SetVSyncEnabled(!G->IsVSyncEnabled());G->ApplySettings(false);return;}
 if(P[0]=="Resolution"){auto G=UGameUserSettings::GetGameUserSettings();G->SetScreenResolution(FIntPoint(A,B));G->ApplySettings(false);return;}
 if(P[0]=="FPS"){auto G=UGameUserSettings::GetGameUserSettings();G->SetFrameRateLimit(A);G->ApplySettings(false);return;}

 if(Id=="New"){M->Screen="Select";M->NewHeroName.Empty();M->NameError.Empty();return;}
 if(P[0]=="Hero"&&M->Classes.IsValidIndex(A)){
  M->NewHeroClass=A;M->Screen="NameHero";M->NameError.Empty();Interface->NameCaret=M->NewHeroName.Len();Interface->NameSelectAll=false;Interface->SetIsFocusable(true);Interface->SetKeyboardFocus();return;
 }
 if(Id=="NameFocus"){Interface->NameCaret=M->NewHeroName.Len();Interface->NameSelectAll=false;Interface->SetIsFocusable(true);Interface->SetKeyboardFocus();return;}
 if(Id=="NameBack"){M->Screen="Select";Interface->SetIsFocusable(false);return;}
 if(Id=="BeginNamed"&&M->Screen=="NameHero"){
  if(M->ValidateCharacterName(M->NewHeroName,M->NameError)&&M->NewGame(M->NewHeroClass,M->HardcoreChoice,M->NewHeroName))Interface->SetIsFocusable(false);return;
 }
 if(Id=="Difficulty"){M->HardcoreChoice=!M->HardcoreChoice;return;}
 if(Id=="Continue"){M->ContinueJourney();return;}
 if(Id=="Load"||Id=="Save"){
  M->BrowserReturn=M->Screen;M->RefreshCharacters();M->CharacterPage=0;
  FString SelectedCharacter=Id=="Save"?S->CharacterName:!S->CharacterName.IsEmpty()?S->CharacterName:M->Characters.IsEmpty()?FString():M->Characters[0].Character;
  M->RefreshSaves(SelectedCharacter);M->Screen=Id=="Save"?"SaveBrowser":"Load";return;
 }
 if(P[0]=="Character"&&M->Characters.IsValidIndex(A)&&M->Screen=="Load"){M->RefreshSaves(M->Characters[A].Character);return;}
 if(P[0]=="CharacterPage"){M->CharacterPage=FMath::Clamp(M->CharacterPage+A,0,FMath::Max(0,(M->Characters.Num()-1)/6));return;}
 if(P[0]=="SavePage"){M->SavePage=FMath::Clamp(M->SavePage+A,0,FMath::Max(0,(M->SaveSlots.Num()-1)/6));return;}
 if(Id=="NewManual"){if(M->Save("Manual")){M->RefreshCharacters();M->RefreshSaves(S->CharacterName);}return;}
 if(Id=="BrowserBack"){M->Screen=M->BrowserReturn;return;}
 if(P[0]=="SavedJourney"&&M->SaveSlots.IsValidIndex(A)){
  const auto& Row=M->SaveSlots[A];if(Row.Ended)return;M->PendingLoadCharacter=Row.Character;M->LoadSlot=Row.Slot;M->LoadLabel=Row.Character+" / "+Row.Label;LoadReturn=M->Screen;M->Screen="LoadConfirm";return;
 }
 if(P[0]=="LoadSlot"&&P.Num()>1){M->PendingLoadCharacter.Empty();M->LoadSlot=P[1];M->LoadLabel=P[1];LoadReturn="Load";M->Screen="LoadConfirm";return;}
 if(Id=="ConfirmLoad"){bool OK=M->PendingLoadCharacter.IsEmpty()?M->Load(M->LoadSlot):M->LoadCharacter(M->PendingLoadCharacter,M->LoadSlot);if(OK)M->PendingLoadCharacter.Empty();return;}
 if(Id=="CancelLoad"){M->PendingLoadCharacter.Empty();M->Screen=LoadReturn;return;}
 if(Id=="IntroNext"){if(++M->IntroPanel>=3)M->Town();return;}
 if(Id=="Back"){if(M->Screen=="Load"||M->Screen=="SaveBrowser"){M->Screen=M->BrowserReturn;return;}M->Screen=S->Party.IsEmpty()?"Menu":S->InTown?"Town":"Dungeon";UIPage=0;M->SelectedItem=-1;return;}
 if(Id=="Resume"){M->Screen=M->PreviousScreen;return;}if(Id=="Menu"){M->Save("Auto");M->RefreshCharacters();M->Screen="Menu";return;}
 if(Id=="Quit"){M->Save("Auto");UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);return;}
 if(Id=="Settings"){SettingsReturn=M->Screen;M->Screen="Settings";return;}if(Id=="SettingsBack"){M->Screen=SettingsReturn;return;}
 if(Id=="Credits"){CreditsFromEnding=false;M->Screen="Credits";return;}if(Id=="EndNext"){CreditsFromEnding=true;M->Screen="Credits";return;}if(Id=="CreditsNext"){if(CreditsFromEnding)M->FinishCredits();else M->Screen="Menu";return;}
 if(Id=="Postgame"){M->BeginPostgame();return;}
 if(P[0]=="Choice"){M->FinalChoice(A==1);return;}
 if(P[0]=="Service"){M->Service=P[1];M->Screen="Service";UIPage=0;SellMode=false;Interface->ShopContext.Empty();return;}
 if(Id=="Town"){M->Town();return;}if(Id=="Enter"){if(S->Floors[0].Shrines.IsEmpty())M->EnterFloor(0);else M->Screen="Waypoints";UIPage=0;return;}
 if(P[0]=="Waypoint"){M->Waypoint(A);return;}if(P[0]=="Buy"){M->Buy(P[1]);return;}
 if(P[0]=="Care"){M->TownService(P[1]);return;}if(P[0]=="Upgrade"){M->Upgrade(M->SelectedHero,A);return;}
 if(P[0]=="Party"){M->SelectedHero=A;M->SelectedItem=-1;return;}
 if(P[0]=="Screen"){M->Screen=P[1];UIPage=0;return;}if(P[0]=="Page"){UIPage=FMath::Max(0,UIPage+A);return;}
 if(P[0]=="Accept"){M->AcceptHunt(A);return;}if(P[0]=="Decline"){M->DeclineHunt(A);return;}if(P[0]=="Claim"){M->ClaimHunt(A);return;}
 if(P[0]=="Item"){M->SelectedHero=A;M->SelectedItem=B;return;}if(Id=="Use"){M->UseItem(M->SelectedHero,M->SelectedItem,M->SelectedHero);return;}
 if(Id=="Equip"){M->Equip(M->SelectedHero,M->SelectedItem);return;}if(P[0]=="EquipAccessory"&&P.Num()==2&&(A==1||A==2)){M->Equip(M->SelectedHero,M->SelectedItem,A);return;}if(P[0]=="Transfer"){M->Transfer(M->SelectedHero,M->SelectedItem,A);return;}
 if(Id=="Drop"){if(S->Party.IsValidIndex(M->SelectedHero)&&S->Party[M->SelectedHero].Bag.IsValidIndex(M->SelectedItem))M->Screen="DropConfirm";return;}
 if(Id=="ConfirmDrop"){S->Party[M->SelectedHero].Bag.RemoveAt(M->SelectedItem);M->SelectedItem=-1;M->Screen="Inventory";return;}
 if(P[0]=="Allocate"){M->Allocate(M->SelectedHero,A);return;}if(P[0]=="Target"){M->Target=A;return;}
 if(P[0]=="Action"){M->Action(P[1],B);return;}
 if(Id=="Fullscreen"){auto Settings=UGameUserSettings::GetGameUserSettings();Settings->SetFullscreenMode(Settings->GetFullscreenMode()==EWindowMode::Windowed?EWindowMode::WindowedFullscreen:EWindowMode::Windowed);Settings->ApplySettings(false);Settings->SaveSettings();return;}
 if(P[0]=="Quality"){auto Settings=UGameUserSettings::GetGameUserSettings();Settings->SetOverallScalabilityLevel(A);Settings->ApplySettings(false);Settings->SaveSettings();return;}
 if(Id=="Audio"){float V=Music->VolumeMultiplier>.01f?0:.16f;Music->SetVolumeMultiplier(V);return;}
}
