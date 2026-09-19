#include "DungeonGame.h"
#include "DungeonRoomKit.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void ADungeonController::StartRoomKitPlaytest(){
 auto M=Model.Get();M->SavePrefix="RoomKitPlaytest_";
 if(!FParse::Param(FCommandLine::Get(),TEXT("RoomKitFresh"))&&M->HasSave("Auto")&&M->Load("Auto")){LastX=-1;return;}
 M->NewGame(0,false);int Floor=8,Seed=int(GetTypeHash(FGuid::NewGuid()));
 FParse::Value(FCommandLine::Get(),TEXT("RoomKitFloor="),Floor);FParse::Value(FCommandLine::Get(),TEXT("RoomKitSeed="),Seed);Floor=FMath::Clamp(Floor,0,M->Floors.Num()-1);
 M->GenerateModularFloor(Floor,Seed);FString Error;
 if(!M->ValidateFloor(Floor,Error)){M->Say("Room generation failed: "+Error);M->Screen="Menu";return;}
 M->Recruit(1);M->Recruit(3);for(auto& Hero:M->State->Party){Hero.Level=FMath::Max(3,M->Floors[Floor].Rank);Hero.Points=Hero.Level*3;Hero.Stats[2]+=Hero.Level;Hero.Stats[4]+=Hero.Level;Hero.HP=M->MaxHP(Hero);Hero.MP=M->MaxMP(Hero);}
 M->State->Gold=450;M->AddItem(FBagItem("health",18));M->AddItem(FBagItem("mana",12));M->EnterFloor(Floor);
 M->Say("Authored-room dungeon playtest. Find the floor key, follow the long guardian tunnel, defeat the boss, then open the descent seal. Saves are separate from your main journey.");
 LastX=-1;UE_LOG(LogTemp,Display,TEXT("ROOM_KIT_PLAYTEST_STARTED floor=%d seed=%d"),Floor,Seed);
}
