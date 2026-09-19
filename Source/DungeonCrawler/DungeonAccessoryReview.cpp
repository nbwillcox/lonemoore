#include "DungeonGame.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

void ADungeonController::TickAccessoryReview(float Delta){
 static int Failures=0;static FString Report;
 ReviewTime+=Delta;if(ReviewTime<3.f)return;ReviewTime=0;auto M=Model.Get();
 const FString Folder=FPaths::ProjectSavedDir()/TEXT("AccessoryFix");IFileManager::Get().MakeDirectory(*Folder,true);
 auto Check=[&](bool OK,FString Text){if(!OK)++Failures;Report+=(OK?"PASS ":"FAIL ")+Text+"\n";UE_LOG(LogTemp,Display,TEXT("ACCESSORY_REVIEW %s %s"),OK?TEXT("PASS"):TEXT("FAIL"),*Text);};
 auto Capture=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(Folder/Name,true,false);};
 switch(ReviewStep++){
 case 0:{M->NewGame(0,false);for(int C:{3,4,5,6})M->Recruit(C);M->Town();M->State->Party[0].Bag={FBagItem("ring",1,2),FBagItem("ring",1,3),FBagItem("charm",1,4)};M->Screen="Inventory";M->SelectedHero=0;M->SelectedItem=0;auto G=UGameUserSettings::GetGameUserSettings();G->SetScreenResolution(FIntPoint(1600,900));G->SetFullscreenMode(EWindowMode::Windowed);G->ApplySettings(false);break;}
 case 1:Check(Interface->Hits.ContainsByPredicate([](const FUIHit& H){return H.Id=="EquipAccessory:1";})&&Interface->Hits.ContainsByPredicate([](const FUIHit& H){return H.Id=="EquipAccessory:2";}),"Both accessory buttons visible");Capture(TEXT("01_accessory_buttons.png"));break;
 case 2:Check(ReviewClick("EquipAccessory:1")&&M->State->Party[0].Gear[6].Quality==2,"Click equips first ring into accessory 1");break;
 case 3:Check(ReviewClick("Item:0:0"),"Select second ring");break;
 case 4:Check(ReviewClick("EquipAccessory:2")&&M->State->Party[0].Gear[6].Quality==2&&M->State->Party[0].Gear[7].Quality==3,"Click equips second ring independently into accessory 2");break;
 case 5:Check(ReviewClick("Item:0:0"),"Select charm");break;
 case 6:{Check(ReviewClick("EquipAccessory:1"),"Click replaces accessory 1");auto& H=M->State->Party[0];Check(H.Gear[6].Id=="charm"&&H.Gear[7].Id=="ring"&&H.Gear[7].Quality==3&&H.Bag[0].Id=="ring"&&H.Bag[0].Quality==2,"Replacement preserves other slot and returns old ring to bag");M->Screen="Character";break;}
 case 7:Capture(TEXT("02_both_accessories_equipped.png"));break;
 default:Report+=FString::Printf(TEXT("Failures: %d\n"),Failures);FFileHelper::SaveStringToFile(Report,*(Folder/TEXT("runtime_results.txt")));UE_LOG(LogTemp,Display,TEXT("ACCESSORY_REVIEW_COMPLETE failures=%d"),Failures);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);break;
 }
}
