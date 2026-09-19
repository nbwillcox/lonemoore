#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DungeonModel.generated.h"

USTRUCT()
struct FBagItem {
 GENERATED_BODY()
 UPROPERTY() FString Id;
 UPROPERTY() int32 Count=1;
 UPROPERTY() int32 Quality=1;
 FBagItem() {} FBagItem(FString InId,int32 N=1,int32 Q=1):Id(InId),Count(N),Quality(Q) {}
};
USTRUCT()
struct FPartyHero {
 GENERATED_BODY()
 UPROPERTY() FString Name;
 UPROPERTY() int32 Class=0;
 UPROPERTY() int32 Level=1;
 UPROPERTY() int32 XP=0;
 UPROPERTY() int32 Points=0;
 UPROPERTY() int32 HP=1;
 UPROPERTY() int32 MP=1;
 UPROPERTY() TArray<int32> Stats;
 UPROPERTY() TArray<FBagItem> Bag;
 UPROPERTY() TArray<FBagItem> Gear;
 UPROPERTY() TMap<FString,int32> Status;
};
USTRUCT()
struct FDungeonBoundary {
 GENERATED_BODY()
 UPROPERTY() FString Id;
 UPROPERTY() int32 A=-1;
 UPROPERTY() int32 B=-1;
 UPROPERTY() FString Kind="Wall";
 UPROPERTY() FString DoorId;
 UPROPERTY() FString Requirement;
 UPROPERTY() int32 ObjectCell=-1;
 UPROPERTY() bool Mandatory=false;
 UPROPERTY() bool SeeThrough=false;
};
USTRUCT()
struct FExpandedRoom {
 GENERATED_BODY()
 UPROPERTY() int32 X=0;
 UPROPERTY() int32 Y=0;
 UPROPERTY() int32 RadiusX=6;
 UPROPERTY() int32 RadiusY=6;
 UPROPERTY() int32 Ceiling=1200;
 UPROPERTY() FString Purpose;
};
USTRUCT()
struct FRoomPlacement {
 GENERATED_BODY()
 UPROPERTY() FString Id;
 UPROPERTY() int32 X=0;
 UPROPERTY() int32 Y=0;
 UPROPERTY() int32 Rotation=0;
 UPROPERTY() int32 ActiveSockets=0;
};
USTRUCT()
struct FFloorRecord {
 GENERATED_BODY()
 UPROPERTY() TArray<FString> Layout;
 UPROPERTY() TArray<int32> Seen;
 UPROPERTY() TArray<int32> Opened;
 UPROPERTY() TArray<int32> Taken;
 UPROPERTY() TMap<int32,int32> Defeated;
 UPROPERTY() TArray<int32> Shrines;
 UPROPERTY() FString FloorId;
 UPROPERTY() int32 ContentVersion=0;
 UPROPERTY() int32 LayoutSeed=0;
 UPROPERTY() TArray<FDungeonBoundary> Boundaries;
 UPROPERTY() TArray<FString> UnlockedDoors;
 UPROPERTY() TArray<FString> OpenDoors;
 UPROPERTY() TArray<FString> Secrets;
 UPROPERTY() TArray<int32> Switches;
 UPROPERTY() TArray<int32> Markers;
 UPROPERTY() TMap<int32,FString> MemoryTiles;
 UPROPERTY() TMap<FString,FString> MemoryEdges;
 UPROPERTY() TArray<int32> ProtectedCells;
 UPROPERTY() TMap<int32,FString> Rooms;
 UPROPERTY() TArray<FExpandedRoom> Volumes;
 UPROPERTY() TArray<FRoomPlacement> RoomPlacements;
 UPROPERTY() TArray<int32> ExitCells;
 UPROPERTY() int32 OriginalArea=0;
 UPROPERTY() int32 OriginalEncounters=0;
 UPROPERTY() int32 EncounterRevision=0;
 UPROPERTY() TMap<int32,int32> EncounterSources;
};
USTRUCT()
struct FHuntRecord {
 GENERATED_BODY()
 UPROPERTY() int32 Serial=0;
 UPROPERTY() FString Name;
 UPROPERTY() FString Family;
 UPROPERTY() int32 Goal=3;
 UPROPERTY() int32 Progress=0;
 UPROPERTY() int32 Gold=60;
 UPROPERTY() int32 XP=50;
 UPROPERTY() int32 Kind=0;
 UPROPERTY() bool Active=false;
 UPROPERTY() bool Hell=false;
 UPROPERTY() int32 TargetFloor=0;
 UPROPERTY() FString Monster;
};
UCLASS()
class UDungeonSave : public USaveGame {
 GENERATED_BODY()
public:
 UPROPERTY() int32 Schema=2;
 UPROPERTY() FString RunId;
 // Empty only for explicit legacy test/review fixtures; real journeys own a named folder.
 UPROPERTY() FString CharacterName;
 UPROPERTY() FString SavedUtc;
 UPROPERTY() FString SavedLabel;
 UPROPERTY() bool Hardcore=false;
 UPROPERTY() bool Dead=false;
 UPROPERTY() bool Postgame=false;
 UPROPERTY() int32 Ending=0;
 UPROPERTY() int32 Floor=0;
 UPROPERTY() int32 X=1;
 UPROPERTY() int32 Y=1;
 UPROPERTY() int32 Facing=0;
 UPROPERTY() int32 Gold=150;
 UPROPERTY() int32 Transitions=0;
 UPROPERTY() int32 HuntSerial=0;
 UPROPERTY() int32 SelectedWaypoint=0;
 UPROPERTY() bool InTown=true;
 UPROPERTY() int32 Meal=0;
 UPROPERTY() TArray<FString> CompanionNames;
 UPROPERTY() TArray<FString> History;
 UPROPERTY() TArray<FBagItem> PendingLoot;
 UPROPERTY() TArray<FPartyHero> Party;
 UPROPERTY() TArray<int32> Recruited;
 UPROPERTY() TArray<FString> Keys;
 UPROPERTY() TArray<int32> Bosses;
 UPROPERTY() TArray<FFloorRecord> Floors;
 UPROPERTY() TArray<FHuntRecord> Hunts;
 UPROPERTY() TArray<FBagItem> Corpse;
 UPROPERTY() int32 CorpseFloor=-1;
 UPROPERTY() int32 CorpseCell=-1;
 UPROPERTY() int32 RandomSeed=73519;
 UPROPERTY() TArray<int32> FallenClasses;
};

struct FClassDef { FString Name,Art,Passive; TArray<int32> Stats; TArray<FString> Skills; TArray<FString> Weapons,Armor; int32 Primary=0; };
struct FSkillDef { FString Name,Type,Effect,Target; int32 Cost=3,Unlock=1; float Power=1; };
struct FItemDef { FString Id,Name,Family,Slot,Effect; int32 Value=10,Power=5,Stack=1; };
struct FEnemyDef { FString Id,Name,Art,Family,Type,Status; float HP=1,Attack=1,Speed=1; TMap<FString,float> Resist; };
struct FFloorDef { FString Name,Region,Boss,Key,RecruitBark,Lore; TArray<FString> Rows,Enemies; int32 RegionIndex=0,Recruit=-1,Rank=1; FLinearColor Tint; };
struct FEnemyUnit { FString Id,Name,Art,Family,Type,Inflict; int32 HP=1,MaxHP=1,Attack=1,Speed=1; TMap<FString,float> Resist; TMap<FString,int32> Status; };
struct FTurnActor { bool Hero=true; int32 Index=0,Speed=0; };
struct FJourneySummary {
 FString Character,Slot,Label,ClassName,Location,UpdatedUtc,RunId;
 int32 Level=1;
 bool Hardcore=false,Ended=false;
};

UCLASS()
class UDungeonModel : public UObject {
 GENERATED_BODY()
public:
 UPROPERTY() TObjectPtr<UDungeonSave> State;
 TArray<FClassDef> Classes;
 TArray<FFloorDef> BaseFloors;
 TArray<FString> CombatCues;
 // Transient presentation events, deliberately excluded from save data.
 struct FCombatEffect { FString Cue; int32 Target=0; bool Hero=false; float Age=0; };
 TArray<FCombatEffect> CombatEffects;
 void CombatFeedback(const FString& Cue,int32 Recipient,bool Hero=false,bool Sound=true);
 void RandomizeLayouts();
 void GenerateFloor(int32 Floor,int32 Seed);
 void GenerateExpandedFloor(int32 Floor,int32 Seed);
 void GenerateModularFloor(int32 Floor,int32 Seed);
 int32 PopulateModularEncounters(int32 Floor,const TArray<int32>& Sources,bool ExistingFloor);
 bool RepairModularEncounters(int32 Floor);
 void BuildModularBoundaries(int32 Floor);
 bool ValidateModularFloor(int32 Floor,FString& Error) const;
 bool UpgradeUnvisitedFloor(int32 Floor);
 void BuildExpandedBoundaries(int32 Floor);
 bool ValidateExpandedFloor(int32 Floor,FString& Error) const;
 void IndexBoundaries(int32 Floor);
 TArray<TMap<uint64,int32>> BoundaryIndices;
 void BuildBoundaries(int32 Floor,bool Legacy=false);
 bool ValidateFloor(int32 Floor,FString& Error) const;
 bool RestoreTopology(FString& Error);
 const FDungeonBoundary* Boundary(int32 Floor,int32 A,int32 B) const;
 bool CanCross(int32 Floor,int32 A,int32 B,bool Sight=false) const;
 bool LineOfSight(int32 Floor,int32 From,int32 To) const;
 bool OpenBoundary(const FDungeonBoundary& Edge);
 void TickDoors(float Delta);
 void UpdateDetection();
 bool HasRequirement(int32 Floor,const FDungeonBoundary& Edge) const;
 TSet<int32> Visible,DetectedEnemies;
 TMap<FString,float> DoorOpening;
 int32 RadarRange=4;
 int32 DiscoveryRevision=0;
 bool CanDescend() const;
 FBagItem EquipmentLoot(int32 Quality);
 FBagItem ChestLoot(int32 Floor,int32 AtCell,bool Sanctuary=false) const;
 int32 SkillDamage(const FPartyHero& Hero,const FSkillDef& Skill) const;
 TArray<FSkillDef> Skills;
 TArray<FItemDef> Items;
 TArray<FEnemyDef> EnemyDefs;
 TArray<FFloorDef> Floors;
 TArray<FEnemyUnit> Enemies;
 TArray<FTurnActor> Order;
 TArray<FString> Log;
 TArray<FString> MaleNames,FemaleNames;
 bool TimedCombat=false;
 float TurnDelay=0;
 int32 PendingEnemy=-1;
 void TickCombat(float Delta);
 TArray<FString> EncounterRoster(int32 AtCell,bool Boss=false) const;
 FString HeroName(const FPartyHero& H) const;
 int32 ItemPower(const FBagItem& B) const;
 FString ItemDetails(const FBagItem& B) const;
 int32 SellPrice(const FBagItem& B) const;
 const FBagItem* SaleItem(int32 Owner,int32 Slot) const;
 bool Sell(int32 Owner,int32 Slot);
 bool NeedsCare(const FString& Action) const;
 void CollectLoot();
 FString Screen="Menu",PreviousScreen="Town",Service="",Notice,LoadSlot="Quick";
 int32 Acting=-1,Turn=0,Round=1,EncounterCell=-1,ReturnX=1,ReturnY=1,Target=0,SelectedHero=0,SelectedItem=-1,IntroPanel=0;
 bool Combat=false,BossFight=false,Betrayal=false,WorldDirty=true,HardcoreChoice=false,Testing=false;
 int32 EncounterHunt=-1;
 FString SavePrefix="Lonemoore_";
 float RegenSeconds=0;
 bool Initialize();
 bool NewGame(int32 ClassIndex,bool Hardcore,const FString& CharacterName=FString());
 bool AllowReviewNames=false;
 FString NewHeroName,NameError,BrowseCharacter,PendingLoadCharacter,LoadLabel,LastSavedSlot,BrowserReturn="Menu";
 int32 NewHeroClass=0,CharacterPage=0,SavePage=0;
 TArray<FJourneySummary> Characters,SaveSlots;
 bool ValidateCharacterName(const FString& Name,FString& Error) const;
 bool CharacterExists(const FString& Name) const;
 FString CharacterRoot() const;
 FString CharacterDirectory(const FString& Name) const;
 FString CharacterSlot(const FString& Name,const FString& Slot) const;
 FString DeathLedgerSlot(const FString& Kind,const UDungeonSave* Journey=nullptr) const;
 bool WriteJourneyMetadata(const FString& Slot);
 void RefreshCharacters();
 void RefreshSaves(const FString& Character);
 bool LoadCharacter(const FString& Character,const FString& Slot);
 bool LoadStoredSlot(const FString& Path,const FString& ExpectedCharacter=FString());
 bool ContinueJourney();
 void Say(const FString& Text);
 const FItemDef* Item(const FString& Id) const;
 const FSkillDef* Skill(const FString& Name) const;
 int32 MaxHP(const FPartyHero& H) const;
 int32 MaxMP(const FPartyHero& H) const;
 int32 Armor(const FPartyHero& H) const;
 int32 Attack(const FPartyHero& H) const;
 int32 Magic(const FPartyHero& H) const;
 int32 NextXP(int32 Level) const;
 void GiveXP(int32 Amount);
 void Recruit(int32 ClassIndex);
 int32 RecruitForFloor(int32 Floor) const;
 bool AddItem(FBagItem Item,int32 Preferred=-1);
 bool UseItem(int32 Owner,int32 Slot,int32 Recipient);
 bool Equip(int32 Owner,int32 Slot,int32 AccessorySlot=0);
 bool Transfer(int32 Owner,int32 Slot,int32 Recipient);
 bool Upgrade(int32 Owner,int32 GearSlot);
 void Allocate(int32 Hero,int32 Attribute);
 int32 Cell(int32 X,int32 Y) const;
 TCHAR Tile(int32 X,int32 Y) const;
 bool Walkable(int32 X,int32 Y) const;
 bool IsDefeated(int32 Cell) const;
 void Reveal();
 bool Move(int32 Direction);
 void Rotate(int32 Direction);
 void Interact();
 void EnterFloor(int32 Floor,int32 Shrine=-1);
 void Town();
 void Waypoint(int32 Packed);
 void TickRecovery(float Delta);
 void StartCombat(int32 AtCell,bool Boss=false,bool Betray=false);
 void BuildOrder();
 void AdvanceTurn();
 void Action(const FString& Action,int32 SkillIndex=-1);
 void HitEnemy(int32 Index,int32 Damage,const FString& Type,const FString& Status="");
 void EnemyAction(int32 Index);
 void FinishCombat();
 void Defeat();
 void RecordHardcoreDeaths();
 int32 Roll(int32 Max);
 void GenerateOffers();
 void HuntProgress(const FString& Family,int32 Kind=0);
 void AcceptHunt(int32 Index);
 void DeclineHunt(int32 Index);
 void ClaimHunt(int32 Index);
 int32 HuntAt(int32 AtCell) const;
 void Buy(const FString& Id);
 void TownService(const FString& Action);
 int32 ServicePrice(const FString& Action) const;
 // Transient UI feedback; never serialized into a player save.
 FString SaveToast;
 double SaveToastUntil=0;
 bool Save(const FString& Slot);
 bool Load(const FString& Slot);
 bool HasSave(const FString& Slot) const;
 void FinalChoice(bool SideWithAstra);
 void FinishCredits();
 void BeginPostgame();
 FString Context() const;
};
