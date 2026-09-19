#include "DungeonGame.h"
#include "DungeonRenderUtils.h"
#include "DungeonPresentation.h"
#include "DungeonCombatAudio.h"
#include "DungeonRegionalArt.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/PointLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
#include "GameFramework/PlayerInput.h"
#include "Misc/ConfigCacheIni.h"

ADungeonGameMode::ADungeonGameMode(){PlayerControllerClass=ADungeonController::StaticClass();DefaultPawnClass=nullptr;HUDClass=nullptr;}
void ADungeonController::BeginPlay(){
 Super::BeginPlay();bShowMouseCursor=true;SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
 Model=NewObject<UDungeonModel>(this);if(!Model->Initialize()){UE_LOG(LogTemp,Error,TEXT("Missing campaign data"));return;}
 FString Raw;TArray<TSharedPtr<FJsonValue>> List;if(FFileHelper::LoadFileToString(Raw,*(FPaths::ProjectContentDir()/TEXT("Game/Data/asset_manifest.json")))&&FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Raw),List)){for(auto V:List){FString Path=V->AsObject()->GetStringField(TEXT("asset"));FString Name=FPaths::GetBaseFilename(Path);int32 Dot=-1;Name.FindChar('.',Dot);if(Dot>=0)Name=Name.Left(Dot);Name.RemoveFromStart("T_");StartupAssets.Add(Path);}}
 TSharedPtr<FJsonObject> Grounding;
 if(FFileHelper::LoadFileToString(Raw,*(FPaths::ProjectContentDir()/TEXT("Game/Data/sprite_grounding.json")))&&FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Raw),Grounding))for(const auto& V:Grounding->GetArrayField(TEXT("sprites"))){const auto O=V->AsObject();SpriteBottomPadding.Add(O->GetStringField(TEXT("art")),O->GetNumberField(TEXT("bottomPadding")));}
 Model->AllowReviewNames=FParse::Param(FCommandLine::Get(),TEXT("RoomKitPlaytest"))||FParse::Param(FCommandLine::Get(),TEXT("ExpansionPlaytest"))||FParse::Param(FCommandLine::Get(),TEXT("RegionalArtPlaytest"))||FParse::Param(FCommandLine::Get(),TEXT("SewerArtPlaytest"));
 // Load this first so the artwork is ready before the first loading frame.
 if(auto T=LoadObject<UTexture2D>(nullptr,TEXT("/Game/Game/ImportedArt/T_loadingscreen.T_loadingscreen")))Textures.Add(TEXT("loadingscreen"),T);
 DungeonCamera=GetWorld()->SpawnActor<ACameraActor>();DungeonCamera->GetCameraComponent()->FieldOfView=76;DungeonCamera->GetCameraComponent()->bConstrainAspectRatio=false;SetViewTarget(DungeonCamera);
 Interface=CreateWidget<UDungeonWidget>(this,UDungeonWidget::StaticClass());Interface->Host=this;Interface->SetVisibility(ESlateVisibility::Visible);Interface->ForceVolatile(true);Interface->AddToViewport();
 Music=NewObject<UAudioComponent>(this);Music->RegisterComponent();Music->bIsUISound=true;LoadPreferences();Music->SetVolumeMultiplier(MasterVolume*MusicVolume);Model->Screen="Loading";Model->TimedCombat=true;
 UGameUserSettings::GetGameUserSettings()->ApplySettings(true);
 ReviewMode=FParse::Param(FCommandLine::Get(),TEXT("InventoryStatusReview"))||FParse::Param(FCommandLine::Get(),TEXT("StorefrontReview"))||FParse::Param(FCommandLine::Get(),TEXT("CombatFXReview"))||FParse::Param(FCommandLine::Get(),TEXT("AdventurePolishReview"))||FParse::Param(FCommandLine::Get(),TEXT("RoomKitBenchmark"))||FParse::Param(FCommandLine::Get(),TEXT("RoomKitReview"))||FParse::Param(FCommandLine::Get(),TEXT("FineTuneReview"))||FParse::Param(FCommandLine::Get(),TEXT("CampaignExpansionReview"))||FParse::Param(FCommandLine::Get(),TEXT("ExpansionReview"))||FParse::Param(FCommandLine::Get(),TEXT("PauseMenuReview"))||FParse::Param(FCommandLine::Get(),TEXT("RegionalArtReview"))||FParse::Param(FCommandLine::Get(),TEXT("SharedPropReview"))||FParse::Param(FCommandLine::Get(),TEXT("SewerArtReview"))||FParse::Param(FCommandLine::Get(),TEXT("AccessoryReview"))||FParse::Param(FCommandLine::Get(),TEXT("IntegrityReview"))||FParse::Param(FCommandLine::Get(),TEXT("MenuArtReview"))||FParse::Param(FCommandLine::Get(),TEXT("DungeonReview"))||FParse::Param(FCommandLine::Get(),TEXT("UIArtReview"));if(ReviewMode){Model->AllowReviewNames=true;Model->Testing=true;Model->SavePrefix="Review_"+FGuid::NewGuid().ToString(EGuidFormats::Digits)+"_";}
}
UTexture2D* ADungeonController::Texture(const FString& Id)const{auto T=Textures.Find(Id);return T?T->Get():nullptr;}
void ADungeonController::PlayCue(const FString& Id){
 auto Sound=CueSounds.FindRef(Id);if(!Sound){Sound=LoadObject<USoundBase>(nullptr,*("/Game/Game/Audio/A_"+Id+".A_"+Id));if(Sound)CueSounds.Add(Id,Sound);}
 if(Sound)UGameplayStatics::PlaySound2D(this,Sound,MasterVolume*EffectsVolume);
 if(FParse::Param(FCommandLine::Get(),TEXT("CombatFXReview")))UE_LOG(LogTemp,Display,TEXT("COMBAT_FX_AUDIO cue=%s loaded=%d volume=%.2f"),*Id,Sound?1:0,MasterVolume*EffectsVolume);
}
void ADungeonController::PlayerTick(float Delta){
 Super::PlayerTick(Delta);if(!Model||!DungeonCamera)return;auto M=Model.Get();PageTurn=FMath::Max(0.f,PageTurn-Delta);
 if(M->Screen=="Loading"){StartupTime+=Delta;if(ReviewMode&&!StartupCaptured&&StartupTime>.2f){StartupCaptured=true;FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/Screenshots/00_loading.png"),true,false);}if(StartupTime>.4f){for(int32 I=0;I<3&&LoadedAssets<StartupAssets.Num();++I){FString Path=StartupAssets[LoadedAssets++];FString Name=FPaths::GetBaseFilename(Path);int32 Dot=-1;Name.FindChar('.',Dot);if(Dot>=0)Name=Name.Left(Dot);Name.RemoveFromStart("T_");if(auto T=LoadObject<UTexture2D>(nullptr,*Path))Textures.Add(Name,T);}}if(LoadedAssets>=StartupAssets.Num()){const auto& Cues=DungeonCombatAudio::CueIds();for(int I=0;I<3&&LoadedCues<Cues.Num();++I){const FString& Id=Cues[LoadedCues++];if(auto Sound=LoadObject<USoundBase>(nullptr,*("/Game/Game/Audio/A_"+Id+".A_"+Id)))CueSounds.Add(Id,Sound);}if(LoadedCues>=Cues.Num()&&StartupTime>1.5f)M->Screen="Menu";}return;}
 if(M->Screen=="NameHero"){if(ReviewMode)TickReview(Delta);return;}
 if(!WorldLoading&&M->WorldDirty&&!M->State->InTown&&!M->State->Party.IsEmpty()&&!ReuseArchitecture())BeginWorldPreparation();
 if(WorldLoading){TickWorldPreparation();return;}
 if(M->Screen=="Menu"&&ReviewStep==0&&FParse::Param(FCommandLine::Get(),TEXT("SewerArtPlaytest"))){ReviewStep=1;StartSewerArtPlaytest();}
 if(M->Screen=="Menu"&&ReviewStep==0&&FParse::Param(FCommandLine::Get(),TEXT("RegionalArtPlaytest"))){ReviewStep=1;StartRegionalArtPlaytest();}
 if(M->Screen=="Menu"&&ReviewStep==0&&FParse::Param(FCommandLine::Get(),TEXT("RoomKitPlaytest"))){ReviewStep=1;StartRoomKitPlaytest();}
 if(M->Screen=="Menu"&&ReviewStep==0&&FParse::Param(FCommandLine::Get(),TEXT("ExpansionPlaytest"))){ReviewStep=1;StartExpansionPlaytest();}
 if(!BindingCapture.IsEmpty()){TArray<FKey> Keys;EKeys::GetAllKeys(Keys);for(auto Key:Keys)if(Key!=EKeys::AnyKey&&Key.IsValid()&&(!Key.IsGamepadKey()&&!Key.IsMouseButton()&&!Key.IsAxis1D()&&!Key.IsAxis2D()&&!Key.IsAxis3D())&&WasInputKeyJustPressed(Key)){if(Key!=EKeys::Escape){FKey Previous=Bindings[BindingCapture];for(auto& Pair:Bindings)if(Pair.Value==Key)Pair.Value=Previous;Bindings[BindingCapture]=Key;SavePreferences();}BindingCapture.Empty();break;}return;}
 if(!PendingCommand.IsEmpty()){if(WasInputKeyJustPressed(EKeys::Escape))PendingCommand.Empty();if(ReviewMode)TickReview(Delta);return;}
 M->TickCombat(Delta);
 if(M->Combat&&M->Screen=="Dungeon"&&!ShowHistory){const FKey Keys[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::F1,EKeys::F2,EKeys::F3};const FString Actions[]={"Attack","Defend","Item","Mana","Flee","Skill:0","Skill:1","Skill:2"};for(int32 I=0;I<8;++I)if(WasInputKeyJustPressed(Keys[I])){Command("Action:"+Actions[I]);break;}}
 for(const auto& Cue:M->CombatCues)PlayCue(Cue);M->CombatCues.Empty();
 if(Pressed("QuickSave")){M->Save("Quick");}
 if(Pressed("QuickLoad")&&!M->Combat&&M->HasSave("Quick")){LoadReturn=M->Screen;M->PendingLoadCharacter.Empty();M->LoadSlot="Quick";M->LoadLabel="Quick";M->Screen="LoadConfirm";}
 if(WasInputKeyJustPressed(EKeys::Escape)){if(M->Screen=="LoadConfirm")Command("CancelLoad");else if(M->Screen=="Load"||M->Screen=="SaveBrowser")Command("BrowserBack");else if(M->Combat)M->Say("Finish the current battle before opening menus.");else if(M->Screen=="Dungeon"||M->Screen=="Town"){M->PreviousScreen=M->Screen;M->Screen="Pause";}else if(M->Screen=="Pause")M->Screen=M->PreviousScreen;else if(M->Screen!="Menu"&&M->Screen!="Choice"&&M->Screen!="Ending")M->Screen=M->State->Party.IsEmpty()?"Menu":M->State->InTown?"Town":"Dungeon";}
 if(M->Screen=="Dungeon"&&!M->Combat&&!ShowHistory&&Tween>=1&&!Pressed("Map")){if(Pressed("Forward")){if(M->Move(1))PlayCue("step");}if(Pressed("Backward")){if(M->Move(-1))PlayCue("step");}if(Pressed("Left"))M->Rotate(-1);if(Pressed("Right"))M->Rotate(1);if(Pressed("Interact")){M->Interact();PlayCue("interact");}if(Pressed("Town"))M->Town();}
 if(M->Screen=="Map"&&Pressed("Map")){M->Screen="Dungeon";return;}
 if(M->Screen=="Inventory"&&!M->Combat&&Pressed("Inventory"))Command("Back");
 else if(M->Screen=="Character"&&!M->Combat&&Pressed("Character"))Command("Back");
 else if((M->Screen=="Town"||M->Screen=="Dungeon")&&!M->Combat){if(Pressed("Map")&&!M->State->InTown){M->Screen="Map";MapFloor=M->State->Floor;MapPan=FVector2D::ZeroVector;}if(Pressed("Inventory"))M->Screen="Inventory";if(Pressed("Character"))M->Screen="Character";if(Pressed("Journal"))M->Screen="Journal";}
 M->TickDoors(Delta);DetectionClock+=Delta;if(DetectionClock>=.2f){DetectionClock=0;M->UpdateDetection();}TickAtmosphere(Delta);
 if(M->WorldDirty){if(!ReuseArchitecture()&&!M->State->InTown&&!M->State->Party.IsEmpty()){BeginWorldPreparation();return;}RebuildWorld();M->WorldDirty=false;}M->TickRecovery(Delta);UpdateView(Delta);
 FString Track=M->Combat?(M->BossFight?"boss":"combat"):M->Screen=="Ending"||M->Screen=="Credits"?"ending":M->State->InTown?"town":"dungeon";
 if(Track!=MusicId){MusicId=Track;if(auto Sound=LoadObject<USoundBase>(nullptr,*("/Game/Game/Audio/A_"+Track+".A_"+Track))){Music->SetSound(Sound);Music->Play();}}
 if(ReviewMode)TickReview(Delta);
}
void ADungeonController::UpdateView(float Delta){
 auto S=Model->State;if(!S||S->Party.IsEmpty())return;
 if(S->X!=LastX||S->Y!=LastY||S->Facing!=LastFacing){CameraFrom=DungeonCamera->GetActorLocation();RotationFrom=DungeonCamera->GetActorRotation();CameraTo=FVector(S->X*400,S->Y*400,155);RotationTo=FRotator(0,(S->Facing-1)*90,0);Tween=LastX<0?1:0;LastX=S->X;LastY=S->Y;LastFacing=S->Facing;}
 Tween=FMath::Min(1.f,Tween+Delta/0.14f);float T=Tween*Tween*(3-2*Tween);DungeonCamera->SetActorLocation(FMath::Lerp(CameraFrom,CameraTo,T));DungeonCamera->SetActorRotation(FMath::Lerp(RotationFrom.Quaternion(),RotationTo.Quaternion(),T));
 const bool TorchVisible=PlayerTorch.IsValid()&&PlayerTorch->PointLightComponent->IsVisible();
 const bool CameraChanged=!LastVisualCamera.Equals(DungeonCamera->GetActorLocation(),.05f)||LastVisualCombat!=Model->Combat||LastTorchVisible!=TorchVisible;
 if(!CameraChanged)return;LastVisualCamera=DungeonCamera->GetActorLocation();LastVisualCombat=Model->Combat;LastTorchVisible=TorchVisible;
 for(auto& K:HoverKeys)if(K.Actor.IsValid()){K.Actor->SetActorHiddenInGame(Model->Combat);auto To=DungeonCamera->GetActorLocation()-K.Actor->GetActorLocation();K.Actor->SetActorRotation(FRotator(0,To.Rotation().Yaw+90,0));}
 for(auto& Enemy:WorldEnemyVisuals)if(Enemy.Actor.IsValid()&&Enemy.Material.IsValid()){
  float Distance=FVector::Distance(DungeonCamera->GetActorLocation(),Enemy.Actor->GetActorLocation());
  float Light=.23f+.55f*FMath::Square(FMath::Clamp(1.f-Distance/2200.f,0.f,1.f));
  if(PlayerTorch.IsValid()&&!PlayerTorch->PointLightComponent->IsVisible())Light=.23f;
  Enemy.Material->SetScalarParameterValue(TEXT("WorldLight"),Light);
 }
 for(auto B:Billboards)if(IsValid(B)){B->SetActorHiddenInGame(Model->Combat);FVector To=DungeonCamera->GetActorLocation()-B->GetActorLocation();B->SetActorRotation(FRotator(0,To.Rotation().Yaw-90,90));}
}
void ADungeonController::TickReview(float Delta){
 if(FParse::Param(FCommandLine::Get(),TEXT("InventoryStatusReview"))){TickInventoryStatusReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("StorefrontReview"))){TickStorefrontReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("CombatFXReview"))){TickCombatFXReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("AdventurePolishReview"))){TickAdventurePolishReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("RoomKitReview"))||FParse::Param(FCommandLine::Get(),TEXT("RoomKitBenchmark"))){TickRoomKitReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("FineTuneReview"))){TickFineTuneReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("CampaignExpansionReview"))){TickCampaignExpansionReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("ExpansionReview"))){TickExpansionReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("RegionalArtReview"))){TickRegionalArtReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("SharedPropReview"))){TickSharedPropReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("SewerArtReview"))){TickSewerArtReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("AccessoryReview"))){TickAccessoryReview(Delta);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("IntegrityReview"))){TickIntegrityReview(Delta);return;}
 if(!PendingCapture.IsEmpty()){CaptureDelay-=Delta;if(CaptureDelay<=0){FString Path=FPaths::ProjectSavedDir()/TEXT("Validation/Screenshots")/(PendingCapture+TEXT(".png"));FScreenshotRequest::RequestScreenshot(Path,true,false);UE_LOG(LogTemp,Display,TEXT("REVIEW_CAPTURE %s"),*PendingCapture);PendingCapture.Empty();}}
 ReviewTime+=Delta;if(ReviewTime<4.f)return;ReviewTime=0;auto M=Model.Get();FString Name;
 if(FParse::Param(FCommandLine::Get(),TEXT("PauseMenuReview"))){
  switch(ReviewStep++){
   case 0:PendingCapture="lonemoore_start_menu";CaptureDelay=1.f;break;
   case 1:M->NewGame(0,false);M->Town();M->PreviousScreen="Town";M->Screen="Pause";PendingCapture="lonemoore_pause_town";CaptureDelay=1.f;break;
   case 2:{const bool Clean=Interface->Hits.Num()==6&&!Interface->Hits.ContainsByPredicate([](const FUIHit& H){return H.Id=="Back";});const bool Resume=ReviewClick("Resume")&&M->Screen=="Town";UE_LOG(LogTemp,Display,TEXT("PAUSE_MENU_REVIEW controls=%s town_resume=%s"),Clean?TEXT("PASS"):TEXT("FAIL"),Resume?TEXT("PASS"):TEXT("FAIL"));if(!Clean||!Resume){FPlatformMisc::RequestExitWithStatus(false,1);return;}M->EnterFloor(0);M->PreviousScreen="Dungeon";M->Screen="Pause";PendingCapture="lonemoore_pause_dungeon";CaptureDelay=1.f;break;}
   case 3:{const bool Resume=ReviewClick("Resume")&&M->Screen=="Dungeon";UE_LOG(LogTemp,Display,TEXT("PAUSE_MENU_REVIEW dungeon_resume=%s"),Resume?TEXT("PASS"):TEXT("FAIL"));FPlatformMisc::RequestExitWithStatus(false,Resume?0:1);break;}
  }return;
 }
 if(FParse::Param(FCommandLine::Get(),TEXT("MenuArtReview"))){switch(ReviewStep++){case 0:PendingCapture="menu_parchment";CaptureDelay=1.5f;break;case 1:ReviewClick("New");PendingCapture="menu_parchment_select";CaptureDelay=1.5f;break;default:UE_LOG(LogTemp,Display,TEXT("MENU_ART_REVIEW_COMPLETE"));FPlatformMisc::RequestExit(false);break;}return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("UIArtReview"))){
  switch(ReviewStep++){
   case 0:M->NewGame(0,false);M->Town();Name="ui_01_town";break;
   case 1:M->Recruit(3);M->Recruit(5);M->Recruit(4);M->Recruit(6);Name="ui_02_full_party";break;
   case 2:ReviewClick("Service:merchant");Name="ui_03_merchant";break;
   case 3:ReviewClick("Screen:Character");Name="ui_04_character";break;
   case 4:Command("Settings");Name="ui_05_settings";break;
   case 5:Command("SettingsBack");M->Town();Command("Enter");M->StartCombat(109,false);Name="ui_06_combat";break;
   case 6:M->Combat=false;M->Town();Command("Service:merchant");Command("Buy:health");Name="ui_07_confirmation";break;
   case 7:ReviewClick("Cancel");Command("Back");break;
   case 8:ReviewClick("History");Name="ui_08_history";break;
   case 9:Command("History");Command("Service:backAlley");Name="ui_09_back_alley";break;
   case 10:ReviewClick("Service:gambler");Name="ui_10_gambler";break;
   case 11:M->State->Party[0].HP=1;M->State->Party[0].Bag.Empty();M->AddItem(FBagItem("health",2),0);Command("Screen:Inventory");Name="ui_11_inventory";break;
   case 12:ReviewClick("Item:0:0",true);UE_LOG(LogTemp,Display,TEXT("REVIEW_DOUBLE_POTION %s"),M->State->Party[0].HP>1&&M->State->Party[0].Bag[0].Count==1?TEXT("PASS"):TEXT("FAIL"));M->EnterFloor(0);M->StartCombat(109);M->Acting=0;M->PendingEnemy=-1;M->TurnDelay=0;InputKey(FInputKeyParams(EKeys::Two,IE_Pressed,1.0));break;
   case 13:InputKey(FInputKeyParams(EKeys::Two,IE_Released,0.0));UE_LOG(LogTemp,Display,TEXT("REVIEW_COMBAT_HOTKEY %s"),M->Log.ContainsByPredicate([](const FString& Line){return Line.Contains("braces for the next attack");})?TEXT("PASS"):TEXT("FAIL"));Name="ui_12_hotkey_combat";break;
   default:UE_LOG(LogTemp,Display,TEXT("UI_ART_REVIEW_COMPLETE"));FPlatformMisc::RequestExit(false);return;
  }
  if(!Name.IsEmpty()){PendingCapture=Name;CaptureDelay=1.5f;}return;
 }
 switch(ReviewStep++){
 case 0:Name="00_menu";break;
 case 1:ReviewClick("New");Name="01_select";break;
 case 2:ReviewClick("Hero:0");M->NewGame(0,false);Interface->SetIsFocusable(false);Name="02_intro";break;
 case 3:Command("IntroNext");Command("IntroNext");Command("IntroNext");Name="03_town";break;
 case 4:ReviewClick("Service:merchant");Name="04_merchant";break;
 case 5:Command("Back");Command("Enter");Name="05_cathedral_entrance";break;
 case 6:M->Move(1);M->Move(1);M->Move(1);M->Move(1);M->Rotate(1);M->Move(1);M->Move(1);M->Rotate(-1);M->Interact();Name="06_open_door";break;
 case 7:M->Screen="Map";Name="07_automap";break;
 case 8:M->Screen="Dungeon";M->StartCombat(109,false);Name="08_first_combat";break;
 case 9:for(int32 I=0;I<50&&M->Combat;++I){if(M->Acting>=0)M->Action("Attack");M->TickCombat(2.f);}M->Screen="Inventory";Name="09_inventory";break;
 case 10:M->Screen="Character";Name="10_character";break;
 case 11:M->Town();M->Recruit(3);M->Recruit(5);M->Recruit(4);M->Recruit(6);Name="11_party_layout_fixture";break;
 case 12:M->Testing=false;M->NewGame(0,false);M->Town();InputKey(FInputKeyParams(EKeys::F5,IE_Pressed,1.0));break;
 case 13:InputKey(FInputKeyParams(EKeys::F5,IE_Released,0.0));UE_LOG(LogTemp,Display,TEXT("REVIEW_F5_ACTIVE %s"),M->HasSave("Quick")?TEXT("PASS"):TEXT("FAIL"));M->PreviousScreen="Town";M->Screen="Pause";M->State->Gold=222;InputKey(FInputKeyParams(EKeys::F5,IE_Pressed,1.0));break;
 case 14:InputKey(FInputKeyParams(EKeys::F5,IE_Released,0.0));M->State->Gold=1;InputKey(FInputKeyParams(EKeys::F9,IE_Pressed,1.0));Name="12_quickload_confirmation";break;
 case 15:InputKey(FInputKeyParams(EKeys::F9,IE_Released,0.0));UE_LOG(LogTemp,Display,TEXT("REVIEW_F9_CONFIRMATION %s"),M->Screen=="LoadConfirm"?TEXT("PASS"):TEXT("FAIL"));ReviewClick("ConfirmLoad");Name="13_restored_journey";break;
 case 16:UE_LOG(LogTemp,Display,TEXT("REVIEW_F5_PAUSED_AND_F9_RESTORE %s"),M->State->Gold==222?TEXT("PASS"):TEXT("FAIL"));for(FString Slot:TArray<FString>{"Auto","Quick","Manual"})UGameplayStatics::DeleteGameInSlot(M->SavePrefix+Slot,0);M->Testing=true;Command("Service:merchant");Command("SellMode");Name="14_pooled_sell";break;
 case 17:ReviewClick("Sell:0:0");Name="15_sale_confirmation";break;
 case 18:ReviewClick("Cancel");Command("Service:blacksmith");Name="16_upgrade_comparison";break;
 case 19:Command("Settings");Name="17_settings_graphics";break;
 case 20:Command("SettingsPage:1");Name="18_settings_audio";break;
 case 21:Command("SettingsPage:2");Name="19_settings_keys";break;
 case 22:Command("SettingsBack");Command("History");Name="20_history";break;
 case 23:Command("History");M->EnterFloor(0);M->State->X=2;M->State->Y=5;M->State->Facing=3;Name="21_key_fixture";break;
 case 24:M->TimedCombat=true;M->StartCombat(109);if(M->Acting>=0)M->Action("Defend");Name="22_timed_combat";CaptureDelay=.2f;break;
 case 25:M->Combat=false;M->Screen="Select";Command("Hero:1");Name="23_class_confirmation";break;
 case 26:Command("Cancel");M->Town();Command("Service:merchant");Command("Buy:health");Name="24_purchase_confirmation";break;
 case 27:ReviewClick("Cancel");Command("SellMode");Command("Sell:0:100");Name="25_equipped_sale_confirmation";break;
 case 28:ReviewClick("Confirm");UE_LOG(LogTemp,Display,TEXT("REVIEW_EQUIPPED_SALE %s"),M->State->Party[0].Gear[0].Id.IsEmpty()?TEXT("PASS"):TEXT("FAIL"));FPlatformMisc::RequestExit(false);return;
 }
 if(!Name.IsEmpty()){PendingCapture=Name;CaptureDelay=1.5f;}
}
void ADungeonController::TickStorefrontReview(float Delta){
 if(!PendingCapture.IsEmpty()){CaptureDelay-=Delta;if(CaptureDelay<=0){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/Storefront")/(PendingCapture+TEXT(".png")),true,false);PendingCapture.Empty();}}
 ReviewTime+=Delta;if(ReviewTime<2.f)return;ReviewTime=0;
 auto M=Model.Get();auto Check=[&](bool OK,const TCHAR* What){UE_LOG(LogTemp,Display,TEXT("STOREFRONT_REVIEW %s %s"),What,OK?TEXT("PASS"):TEXT("FAIL"));if(!OK)++StorefrontFailures;};
 auto Click=[&](const FString& Id){Check(ReviewClick(Id),*Id);};
 auto Capture=[&](const FString& Name){PendingCapture=Name;CaptureDelay=.7f;};
 auto Action=[&](const FString& Prefix){const FUIHit* Hit=Interface->Hits.FindByPredicate([&](const FUIHit& H){return H.Id.StartsWith(Prefix);});StorefrontAction=Hit?Hit->Id:FString();Check(Hit!=nullptr,*("action_present_"+Prefix));};
 auto Service=[&](const FString& Id){Command("Service:"+Id);Interface->Mouse=FVector2D(-100,-100);Capture(Id);};
 switch(ReviewStep++){
 case 0:M->NewGame(0,false);M->Town();M->State->Gold=10000;M->State->Party[0].HP=1;M->State->Party[0].MP=0;Check(M->Testing&&M->SavePrefix.StartsWith("Review_"),TEXT("isolated_fixture"));Service("tavern");break;
 case 1:{const auto* Hit=Interface->Hits.FindByPredicate([](const FUIHit& H){return H.Id=="ShopSelect:2";});Check(Hit!=nullptr,TEXT("meal_row"));if(Hit){const auto G=Interface->GetCachedGeometry();const float Scale=FMath::Min(G.GetLocalSize().X/1600.f,G.GetLocalSize().Y/900.f);const auto Absolute=G.LocalToAbsolute((G.GetLocalSize()-FVector2D(1600,900)*Scale)/2+Hit->Bounds.GetCenter()*Scale);FPointerEvent E(0,Absolute,Absolute,TSet<FKey>(),EKeys::Invalid,0,FModifierKeysState());Interface->NativeOnMouseMove(G,E);}Capture("tavern_hover_meal");break;}
 case 2:Check(Interface->Hits.ContainsByPredicate([](const FUIHit& H){return H.Id=="Care:meal";}),TEXT("hover_displays_meal_details"));Click("ShopSelect:2");StorefrontGold=M->State->Gold;break;
 case 3:Click("Care:meal");Check(M->State->Meal==3&&M->State->Gold==StorefrontGold-M->ServicePrice("meal"),TEXT("meal_purchase"));Service("healer");break;
 case 4:Click("ShopSelect:0");break;
 case 5:Click("Care:heal");Check(M->State->Party[0].HP==M->MaxHP(M->State->Party[0]),TEXT("heal_purchase"));Service("merchant");break;
 case 6:Click("ShopSelect:0");break;
 case 7:Action("Buy:");StorefrontGold=M->State->Gold;if(!StorefrontAction.IsEmpty())Click(StorefrontAction);Check(!PendingCommand.IsEmpty(),TEXT("buy_confirmation"));Capture("merchant_buy_confirmation");break;
 case 8:Click("Cancel");Check(M->State->Gold==StorefrontGold&&PendingCommand.IsEmpty(),TEXT("buy_cancel_preserves_gold"));break;
 case 9:if(!StorefrontAction.IsEmpty())Click(StorefrontAction);break;
 case 10:{const auto* D=M->Item(StorefrontAction.Mid(4));const int32 Price=D?D->Value:0;Click("Confirm");Check(M->State->Gold==StorefrontGold-Price&&Price>0,TEXT("buy_confirm_debits_price"));break;}
 case 11:Click("Page:1");Capture("merchant_next_page");break;
 case 12:Check(UIPage==1,TEXT("merchant_pagination"));Click("SellMode");Capture("merchant_sell");break;
 case 13:Click("ShopSelect:0");break;
 case 14:Action("Sell:");StorefrontGold=M->State->Gold;if(!StorefrontAction.IsEmpty())Click(StorefrontAction);Capture("merchant_sell_confirmation");break;
 case 15:Click("Confirm");Check(M->State->Gold>StorefrontGold,TEXT("sale_credits_gold"));Service("blacksmith");break;
 case 16:Click("ShopSelect:0");break;
 case 17:Action("Upgrade:");if(!StorefrontAction.IsEmpty()){const int32 Slot=FCString::Atoi(*StorefrontAction.Mid(8));StorefrontQuality=M->State->Party[0].Gear[Slot].Quality;StorefrontGold=M->State->Gold;Click(StorefrontAction);Check(M->State->Party[0].Gear[Slot].Quality==StorefrontQuality+1&&M->State->Gold<StorefrontGold,TEXT("upgrade_changes_quality_and_gold"));}Capture("blacksmith_upgraded");break;
 case 18:Service("gambler");break;
 case 19:Click("ShopSelect:0");break;
 case 20:StorefrontGold=M->State->Gold;Click("Care:gamble");Capture("gambler_confirmation");break;
 case 21:Click("Confirm");Check(M->State->Gold==StorefrontGold-M->ServicePrice("gamble"),TEXT("gambler_purchase"));Service("backAlley");break;
 case 22:Service("huntBoard");break;
 case 23:Service("dungeonEntrance");break;
 case 24:M->State->Gold=0;Service("merchant");break;
 case 25:Check(!Interface->Hits.ContainsByPredicate([](const FUIHit& H){return H.Id.StartsWith("Buy:");}),TEXT("unaffordable_purchase_disabled"));Capture("merchant_unaffordable");break;
 case 26:Click("History");Check(ShowHistory,TEXT("history_open"));Capture("merchant_history");break;
 case 27:Click("History");Check(!ShowHistory,TEXT("history_close"));M->State->Gold=10000;for(auto& B:M->State->Party[0].Gear)if(!B.Id.IsEmpty())B.Quality=4;Service("blacksmith");break;
 case 28:Check(!Interface->Hits.ContainsByPredicate([](const FUIHit& H){return H.Id.StartsWith("Upgrade:");}),TEXT("maximum_quality_upgrade_disabled"));Capture("blacksmith_maximum_quality");break;
 case 29:M->State->Party[0].Bag.Empty();Service("merchant");Command("SellMode");Capture("merchant_equipped_items");break;
 case 30:Click("ShopSelect:0");break;
 case 31:Action("Sell:");StorefrontGold=M->State->Gold;if(!StorefrontAction.IsEmpty())Click(StorefrontAction);Check(ConfirmText.Contains("equipped"),TEXT("equipped_sale_warning"));Capture("merchant_equipped_sale_confirmation");break;
 case 32:Click("Cancel");Check(M->State->Gold==StorefrontGold&&PendingCommand.IsEmpty(),TEXT("equipped_sale_cancel_preserves_gold"));break;
 case 33:if(!StorefrontAction.IsEmpty())Click(StorefrontAction);break;
 case 34:{TArray<FString> Parts;StorefrontAction.ParseIntoArray(Parts,TEXT(":"));const int32 Slot=Parts.Num()==3?FCString::Atoi(*Parts[2])-100:-1;Click("Confirm");Check(Slot>=0&&M->State->Party[0].Gear.IsValidIndex(Slot)&&M->State->Party[0].Gear[Slot].Id.IsEmpty()&&M->State->Gold>StorefrontGold,TEXT("equipped_sale_removes_gear"));break;}
 case 35:Check(M->Testing&&M->SavePrefix.StartsWith("Review_"),TEXT("isolation_preserved"));UE_LOG(LogTemp,Display,TEXT("STOREFRONT_REVIEW_COMPLETE failures=%d"),StorefrontFailures);FPlatformMisc::RequestExitWithStatus(false,StorefrontFailures?1:0);break;
 }
}
bool ADungeonController::ReviewClick(const FString& Id,bool Double){
 auto Hit=Interface->Hits.FindByPredicate([&](const FUIHit& H){return H.Id==Id;});if(!Hit||Interface->GetVisibility()!=ESlateVisibility::Visible){UE_LOG(LogTemp,Error,TEXT("REVIEW_CLICK_FAILED %s"),*Id);return false;}
 FGeometry G=Interface->GetCachedGeometry();float S=FMath::Min(G.GetLocalSize().X/1600.f,G.GetLocalSize().Y/900.f);FVector2D Local=(G.GetLocalSize()-FVector2D(1600,900)*S)/2+Hit->Bounds.GetCenter()*S;FVector2D Absolute=G.LocalToAbsolute(Local);TSet<FKey> Buttons;Buttons.Add(EKeys::LeftMouseButton);
 FPointerEvent Event(0,Absolute,Absolute,Buttons,EKeys::LeftMouseButton,0,FModifierKeysState());bool Handled=(Double?Interface->NativeOnMouseButtonDoubleClick(G,Event):Interface->NativeOnMouseButtonDown(G,Event)).IsEventHandled();UE_LOG(LogTemp,Display,TEXT("REVIEW_CLICK %s %s"),*Id,Handled?TEXT("PASS"):TEXT("FAIL"));return Handled;
}
bool ADungeonController::ReuseArchitecture() const{
 auto S=Model->State;if(!S||S->InTown||S->Party.IsEmpty()||Architecture.IsEmpty()||BuiltState.Get()!=S||BuiltFloor!=S->Floor)return false;
 const auto& R=S->Floors[S->Floor];return BuiltSeed==R.LayoutSeed&&BuiltVersion==R.ContentVersion;
}
void ADungeonController::RebuildWorld(){
 const bool KeepArchitecture=ReuseArchitecture();const double Started=FPlatformTime::Seconds();
 for(auto A:Scenery)if(IsValid(A)&&(!KeepArchitecture||!Architecture.Contains(A)))A->Destroy();
 if(!KeepArchitecture){Architecture.Empty();DoorVisuals.Empty();Flames.Empty();Dust=nullptr;DustOrigins.Empty();BuiltState.Reset();BuiltFloor=-1;}
 Scenery=Architecture;Billboards.Empty();HoverKeys.Empty();WorldEnemyVisuals.Empty();PlayerTorch.Reset();PlayerTorchFill.Reset();LastVisualCamera=FVector(1.e20);auto S=Model->State;if(S->Party.IsEmpty()||S->InTown){LastX=-1;return;}
 if(!KeepArchitecture)LastX=-1;
 auto& D=Model->Floors[S->Floor];auto& R=S->Floors[S->Floor];auto Root=GetWorld()->SpawnActor<AActor>();Scenery.Add(Root);auto RC=NewObject<USceneComponent>(Root);Root->SetRootComponent(RC);RC->RegisterComponent();
 TMap<FString,UInstancedStaticMeshComponent*> Batches;
 auto Mesh=[&](FString Name,FVector P,FRotator Rot=FRotator::ZeroRotator,FVector Scale=FVector::OneVector,FString Material="Stone"){
  FString Key=Name+Material;auto Found=Batches.Find(Key);UInstancedStaticMeshComponent* C=Found?*Found:nullptr;
  if(!C){bool Shared=Name=="SharedCrate"||Name=="SharedLever",Stair=Name.StartsWith("Stair_");FString MeshRoot=Stair?"/Game/CampaignExpansion/Meshes/SM_":Shared?"/Game/SharedInteractables/Meshes/SM_":"/Game/Game/Environment/SM_";auto SM=LoadObject<UStaticMesh>(nullptr,*(Name=="SharedLever"?RegionalArt::LeverFor(D.RegionIndex):MeshRoot+Name+".SM_"+Name));if(!SM){UE_LOG(LogTemp,Error,TEXT("Missing world mesh %s"),*Name);return;}C=NewObject<UInstancedStaticMeshComponent>(Root);C->SetStaticMesh(SM);C->SetupAttachment(RC);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->RegisterComponent();auto Base=LoadObject<UMaterialInterface>(nullptr,*("/Game/Game/Materials/M_"+Material+".M_"+Material));if(Base&&!Shared&&!Stair){auto MI=UMaterialInstanceDynamic::Create(Base,C);if(Material=="Stone")MI->SetVectorParameterValue("Tint",D.Tint);C->SetMaterial(0,MI);}Batches.Add(Key,C);}
  C->AddInstance(FTransform(Rot,P,Scale));
 };
 auto Sprite=[&](FString Art,FVector P,float Width,int32 KeyColumn=-1){bool WorldEnemy=Model->EnemyDefs.ContainsByPredicate([&](const FEnemyDef& E){return E.Art==Art;});auto Tex=Texture(Art);auto Plane=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Plane.Plane"));auto Base=LoadObject<UMaterialInterface>(nullptr,KeyColumn>=0?TEXT("/Game/Game/Materials/M_Key.M_Key"):WorldEnemy?TEXT("/Game/ExpansionPrototype/Materials/M_ExpansionEnemy.M_ExpansionEnemy"):TEXT("/Game/Game/Materials/M_Billboard.M_Billboard"));if(!Tex||!Plane||!Base)return;if(WorldEnemy){const float MaxEnemyHeight=Model->State->Floors[Model->State->Floor].ContentVersion>=3?520.f:440.f;Width=FMath::Min(Width*2.f,MaxEnemyHeight*float(Tex->GetSizeX())/Tex->GetSizeY());}auto A=GetWorld()->SpawnActor<AStaticMeshActor>();A->SetMobility(EComponentMobility::Movable);A->SetActorEnableCollision(false);A->GetStaticMeshComponent()->SetStaticMesh(Plane);A->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);auto MI=UMaterialInstanceDynamic::Create(Base,A);MI->SetTextureParameterValue("Portrait",Tex);if(KeyColumn>=0)MI->SetScalarParameterValue("Column",KeyColumn*.2f);A->GetStaticMeshComponent()->SetMaterial(0,MI);A->GetStaticMeshComponent()->SetCastShadow(false);A->GetStaticMeshComponent()->SetCullDistance(16000);float Height=KeyColumn>=0?Width*2.3f:Width*float(Tex->GetSizeY())/Tex->GetSizeX();A->SetActorLocation(P+FVector(0,0,WorldEnemy?DungeonPresentation::GroundedEnemyCenter(Height,SpriteBottomPadding.FindRef(Art)):Height/2));if(WorldEnemy){A->Tags.Add("DungeonEnemy");A->Tags.Add(FName(*Art));}if(Art=="waypoint_shrine")A->Tags.Add("DungeonWaypoint");A->SetActorScale3D(FVector(Width/100,Height/100,1));Scenery.Add(A);Billboards.Add(A);if(WorldEnemy)WorldEnemyVisuals.Add({A,MI});if(KeyColumn>=0){FVector Origin=A->GetActorLocation()+FVector(0,0,18);HoverKeys.Add({A,Origin,float(P.X*.003+P.Y*.007)});A->SetActorLocation(Origin);}};
 const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
 for(int32 Y=0;Y<D.Rows.Num();++Y)for(int32 X=0;X<D.Rows[Y].Len();++X){TCHAR T=Model->Tile(X,Y);if(T=='#')continue;int32 C=Model->Cell(X,Y);FVector P(X*400,Y*400,0);
  if((T=='C'||T=='$')&&!R.Taken.Contains(C))Mesh("SharedCrate",P,FRotator::ZeroRotator,FVector::OneVector,"Wood");
  if(T=='K'&&!R.Taken.Contains(C)){
   auto SM=LoadObject<UStaticMesh>(nullptr,*RegionalArt::KeyFor(D.Key));if(SM){auto A=GetWorld()->SpawnActor<AStaticMeshActor>();A->SetMobility(EComponentMobility::Movable);A->SetActorEnableCollision(false);auto MC=A->GetStaticMeshComponent();MC->SetStaticMesh(SM);MC->SetCollisionEnabled(ECollisionEnabled::NoCollision);MC->SetCastShadow(false);FVector Origin=P+FVector(0,0,110);A->SetActorLocation(Origin);A->SetActorScale3D(FVector(1.4));Scenery.Add(A);HoverKeys.Add({A,Origin,float(P.X*.003+P.Y*.007)});}else UE_LOG(LogTemp,Error,TEXT("Missing regional key model %s"),*D.Key);
  }
  if(T=='L')Mesh("SharedLever",P,FRotator::ZeroRotator,FVector::OneVector,"Metal");
  if((T=='T'&&!R.Taken.Contains(C))||T=='P')Mesh("PressurePlate",P,FRotator::ZeroRotator,FVector::OneVector,"Metal");
  if(T=='>'||T=='A')Mesh(FString::Printf(TEXT("Stair_%02d"),S->Floor),P,FRotator(0,180,0));
  if(T=='S'||T=='V'){const FVector ShrineP=P;Sprite("waypoint_shrine",ShrineP,180);auto L=GetWorld()->SpawnActor<APointLight>(ShrineP+FVector(0,0,190),FRotator::ZeroRotator);L->SetMobility(EComponentMobility::Movable);L->PointLightComponent->SetIntensity(2200);L->PointLightComponent->SetAttenuationRadius(650);L->PointLightComponent->SetLightColor(FLinearColor(.2,.65,1));L->PointLightComponent->SetCastShadows(false);ConfigureDungeonLight(L->PointLightComponent);Scenery.Add(L);}
  if((T=='E'||T=='B'||T=='m')&&!Model->IsDefeated(C)){auto Roster=Model->EncounterRoster(C,T!='E');for(int32 I=0;I<Roster.Num();++I){auto Def=Model->EnemyDefs.FindByPredicate([&](const FEnemyDef& E){return E.Id==Roster[I];});if(Def){float Offset=(I-(Roster.Num()-1)*.5f)*(360.f/FMath::Max(1,Roster.Num()));Sprite(Def->Art,P+FVector(Offset,Offset,0),Roster.Num()>1?100:160);}}}
  if(T=='A'&&!S->Postgame)Sprite("final_boss",P,240);
  if(T=='H'){int32 Hunt=Model->HuntAt(C);if(Hunt>=0){FString Id=S->Hunts[Hunt].Monster;auto Def=Model->EnemyDefs.FindByPredicate([&](const FEnemyDef& E){return E.Id==Id;});if(Def)Sprite(Def->Art,P,210);}}
  if(T=='R'&&!R.Taken.Contains(C)&&Model->RecruitForFloor(S->Floor)>=0&&!S->Recruited.Contains(Model->RecruitForFloor(S->Floor)))Sprite(Model->Classes[Model->RecruitForFloor(S->Floor)].Art,P,100);

 }
 if(!KeepArchitecture){int First=Scenery.Num();BuildArchitecture();for(int I=First;I<Scenery.Num();++I)Architecture.Add(Scenery[I]);BuiltState=S;BuiltFloor=S->Floor;BuiltSeed=R.LayoutSeed;BuiltVersion=R.ContentVersion;++ArchitectureBuilds;}
 UE_LOG(LogTemp,Display,TEXT("DUNGEON_WORLD_REFRESH architecture=%s build_count=%d cpu_ms=%.3f"),KeepArchitecture?TEXT("reused"):TEXT("built"),ArchitectureBuilds,(FPlatformTime::Seconds()-Started)*1000);
 // Soft player lantern keeps navigable stone readable without illuminating distant rooms.
 auto Lantern=GetWorld()->SpawnActor<APointLight>();Lantern->SetMobility(EComponentMobility::Movable);Lantern->AttachToActor(DungeonCamera,FAttachmentTransformRules::KeepRelativeTransform);Lantern->SetActorRelativeLocation(FVector(30,28,12));Lantern->PointLightComponent->SetIntensity(DungeonPresentation::PlayerTorchIntensity*Brightness);Lantern->PointLightComponent->SetAttenuationRadius(1850);Lantern->PointLightComponent->SetLightColor(FLinearColor(1.f,.68f,.34f));Lantern->PointLightComponent->SetCastShadows(false);Scenery.Add(Lantern);
 PlayerTorch=Lantern;auto Fill=GetWorld()->SpawnActor<APointLight>();Fill->SetMobility(EComponentMobility::Movable);Fill->AttachToActor(DungeonCamera,FAttachmentTransformRules::KeepRelativeTransform);Fill->SetActorRelativeLocation(FVector(-45,0,50));Fill->PointLightComponent->SetIntensity(DungeonPresentation::PlayerFillIntensity*Brightness);Fill->PointLightComponent->SetAttenuationRadius(1400);Fill->PointLightComponent->SetLightColor(FLinearColor(.78f,.64f,.47f));Fill->PointLightComponent->SetCastShadows(false);Scenery.Add(Fill);PlayerTorchFill=Fill;

}

bool ADungeonController::Pressed(const FString& Action)const{const FKey* Key=Bindings.Find(Action);return Key&&WasInputKeyJustPressed(*Key);}
void ADungeonController::LoadPreferences(){
 const FString Actions[]={"Forward","Backward","Left","Right","Interact","Town","Map","Inventory","Character","Journal","QuickSave","QuickLoad"};
 const FKey Keys[]={EKeys::W,EKeys::S,EKeys::A,EKeys::D,EKeys::E,EKeys::T,EKeys::M,EKeys::I,EKeys::C,EKeys::J,EKeys::F5,EKeys::F9};
 for(int32 I=0;I<12;++I){FString Name=Keys[I].ToString();GConfig->GetString(TEXT("DungeonPreferences"),*Actions[I],Name,GGameUserSettingsIni);FKey Key(*Name);Bindings.Add(Actions[I],Key.IsValid()?Key:Keys[I]);}
 GConfig->GetInt(TEXT("DungeonPreferences"),TEXT("RadarRange"),Model->RadarRange,GGameUserSettingsIni);Model->RadarRange=FMath::Clamp(Model->RadarRange,2,6);
 GConfig->GetFloat(TEXT("DungeonPreferences"),TEXT("Master"),MasterVolume,GGameUserSettingsIni);GConfig->GetFloat(TEXT("DungeonPreferences"),TEXT("Music"),MusicVolume,GGameUserSettingsIni);GConfig->GetFloat(TEXT("DungeonPreferences"),TEXT("Effects"),EffectsVolume,GGameUserSettingsIni);GConfig->GetFloat(TEXT("DungeonPreferences"),TEXT("Brightness"),Brightness,GGameUserSettingsIni);
 MasterVolume=FMath::Clamp(MasterVolume,0.f,1.f);MusicVolume=FMath::Clamp(MusicVolume,0.f,1.f);EffectsVolume=FMath::Clamp(EffectsVolume,0.f,1.f);Brightness=FMath::Clamp(Brightness,.5f,2.f);
}
void ADungeonController::SavePreferences(){GConfig->SetInt(TEXT("DungeonPreferences"),TEXT("RadarRange"),Model->RadarRange,GGameUserSettingsIni);for(auto Pair:Bindings)GConfig->SetString(TEXT("DungeonPreferences"),*Pair.Key,*Pair.Value.ToString(),GGameUserSettingsIni);GConfig->SetFloat(TEXT("DungeonPreferences"),TEXT("Master"),MasterVolume,GGameUserSettingsIni);GConfig->SetFloat(TEXT("DungeonPreferences"),TEXT("Music"),MusicVolume,GGameUserSettingsIni);GConfig->SetFloat(TEXT("DungeonPreferences"),TEXT("Effects"),EffectsVolume,GGameUserSettingsIni);GConfig->SetFloat(TEXT("DungeonPreferences"),TEXT("Brightness"),Brightness,GGameUserSettingsIni);GConfig->Flush(false,GGameUserSettingsIni);Music->SetVolumeMultiplier(MasterVolume*MusicVolume);}

