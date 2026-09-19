#include "DungeonGame.h"
#include "GameFramework/PlayerInput.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

void ADungeonController::TickInventoryStatusReview(float Delta){
 static int32 Failures=0;
 ReviewTime+=Delta;if(ReviewTime<3.f)return;ReviewTime=0;auto M=Model.Get();
 const FString Folder=FPaths::ProjectSavedDir()/TEXT("InventoryStatusReview");IFileManager::Get().MakeDirectory(*Folder,true);
 auto Check=[&](bool OK,const TCHAR* Name){if(!OK)++Failures;UE_LOG(LogTemp,Display,TEXT("INVENTORY_STATUS_REVIEW %s %s"),OK?TEXT("PASS"):TEXT("FAIL"),Name);};
 auto Capture=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(Folder/Name,true,false);};
 switch(ReviewStep++){
 case 0:M->NewGame(0,false);M->Town();M->State->Gold=10000;for(int C:{3,4,5,6})M->Recruit(C);M->EnterFloor(0);break;
 case 1:{Check(M->State->Party.Num()==5,TEXT("five heroes"));M->Screen="Inventory";M->SelectedHero=0;M->SelectedItem=0;M->State->Party[0].Bag={FBagItem("health",2),FBagItem("ring",1,2),FBagItem("charm",1,3)};M->State->Party[0].HP=1;const TCHAR* Statuses[]={TEXT("Poison"),TEXT("Bleed"),TEXT("Plague"),TEXT("Slow"),TEXT("Silence")};for(int I=0;I<M->State->Party.Num();++I){M->State->Party[I].Status.Add(Statuses[I],3);if(I>0)M->State->Party[I].Bag={FBagItem("health",2),FBagItem("mana",2)};}break;}
 case 2:Check(Interface->Hits.ContainsByPredicate([](const FUIHit& H){return H.Id=="Back";}),TEXT("close control"));Check(!Interface->Hits.ContainsByPredicate([](const FUIHit& H){return H.Id=="History";}),TEXT("notice feed hidden"));Check(Interface->Hits.ContainsByPredicate([](const FUIHit& H){return H.Id=="Item:4:0";}),TEXT("all five inventories"));Capture(TEXT("01_inventory_five.png"));break;
 case 3:Check(ReviewClick("Use")&&M->State->Party[0].HP>1&&M->State->Party[0].Bag[0].Count==1,TEXT("use potion"));ReviewClick("Item:0:1");break;
 case 4:Check(ReviewClick("EquipAccessory:1")&&M->State->Party[0].Gear[6].Id=="ring",TEXT("equip accessory one"));ReviewClick("Item:0:1");break;
 case 5:Check(ReviewClick("Transfer:1"),TEXT("transfer item"));Check(M->State->Party[1].Bag.ContainsByPredicate([](const FBagItem& I){return I.Id=="charm";}),TEXT("recipient receives item"));ReviewClick("Item:0:0");ReviewClick("Drop");break;
 case 6:Check(M->Screen=="DropConfirm",TEXT("drop confirmation"));Capture(TEXT("02_drop_confirm.png"));ReviewClick("Screen:Inventory");break;
 case 7:Check(M->Screen=="Inventory",TEXT("cancel preserves popup"));Check(ReviewClick("Party:4")&&M->SelectedHero==4&&M->Screen=="Inventory",TEXT("party card selection"));Check(ReviewClick("Back")&&M->Screen=="Dungeon",TEXT("close returns to dungeon"));break;
 case 8:Capture(TEXT("03_dungeon_status_cards.png"));M->State->Party.SetNum(1);M->Screen="Inventory";M->SelectedHero=0;break;
 case 9:Capture(TEXT("04_inventory_solo.png"));InputKey(FInputKeyParams(EKeys::I,IE_Pressed,1.0));break;
 case 10:InputKey(FInputKeyParams(EKeys::I,IE_Released,0.0));Check(M->Screen=="Dungeon",TEXT("I key closes bag"));InputKey(FInputKeyParams(EKeys::I,IE_Pressed,1.0));break;
 case 11:InputKey(FInputKeyParams(EKeys::I,IE_Released,0.0));Check(M->Screen=="Inventory",TEXT("I key opens bag"));for(const TCHAR* Key:{TEXT("Poison"),TEXT("Bleed"),TEXT("Burn"),TEXT("Plague"),TEXT("Slow"),TEXT("Weakness"),TEXT("Silence"),TEXT("Stun"),TEXT("Fear"),TEXT("Blind"),TEXT("Curse"),TEXT("Mark"),TEXT("Guard")})M->State->Party[0].Status.Add(Key,3);break;
 case 12:Capture(TEXT("05_all_status_badges.png"));InputKey(FInputKeyParams(EKeys::Escape,IE_Pressed,1.0));break;
 case 13:InputKey(FInputKeyParams(EKeys::Escape,IE_Released,0.0));Check(M->Screen=="Dungeon",TEXT("Escape closes bag"));break;
 default:UE_LOG(LogTemp,Display,TEXT("INVENTORY_STATUS_REVIEW_COMPLETE failures=%d"),Failures);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);break;
 }
}
