#include "DungeonModel.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace {
UDungeonModel* Fresh(int32 Class=0,bool Hardcore=false){auto M=NewObject<UDungeonModel>();M->Initialize();M->Testing=true;M->NewGame(Class,Hardcore);M->EnterFloor(0);return M;}
void Fight(UDungeonModel* M){for(int32 I=0;I<500&&M->Combat;++I){auto H=M->State->Party[M->Acting];if(H.HP<M->MaxHP(H)/3&&H.Bag.ContainsByPredicate([](const FBagItem& B){return B.Id=="health"||B.Id=="greater_health";}))M->Action("Item");else if(!H.Status.Contains("Silence")&&H.MP>=4)M->Action("Skill",0);else M->Action("Attack");}}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonCore,"Dungeon.Core.Rules",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonCore::RunTest(const FString& Parameters){
 auto M=Fresh();TestEqual(TEXT("Seven classes"),M->Classes.Num(),7);TestEqual(TEXT("Exactly 21 active skills"),M->Skills.Num(),21);TestEqual(TEXT("One starting hero"),M->State->Party.Num(),1);
 for(auto C:M->Classes){TestEqual(TEXT("Three skills per class"),C.Skills.Num(),3);TestEqual(TEXT("Six attributes"),C.Stats.Num(),6);for(int32 I=0;I<3;++I){auto D=M->Skill(C.Skills[I]);TestNotNull(TEXT("Skill exists"),D);if(D)TestEqual(TEXT("Correct unlock"),D->Unlock,I==0?1:I==1?5:10);}}
 const int32 StartX=M->State->X,StartY=M->State->Y,StartFacing=M->State->Facing;const int32 StepX[]={0,1,0,-1},StepY[]={-1,0,1,0};
 TestTrue(TEXT("Forward step succeeds through the connected entrance"),M->Move(1));TestEqual(TEXT("One grid step follows actual facing on X"),M->State->X,StartX+StepX[StartFacing]);TestEqual(TEXT("One grid step follows actual facing on Y"),M->State->Y,StartY+StepY[StartFacing]);for(int32 I=0;I<4;++I)M->Rotate(1);TestEqual(TEXT("Four turns preserve facing"),M->State->Facing,StartFacing);
 auto Wall=M->State->Floors[0].Boundaries.FindByPredicate([](const FDungeonBoundary& E){return E.Kind=="Wall"&&E.B>=0;});int W=M->Floors[0].Rows[0].Len();
 auto Face=[&](int A,int B){M->State->X=A%W;M->State->Y=A/W;M->State->Facing=B%W>A%W?1:B%W<A%W?3:B/W>A/W?2:0;};
 if(Wall){Face(Wall->A,Wall->B);TestFalse(TEXT("Wall blocks movement"),M->Move(1));}
 auto Gate=M->State->Floors[0].Boundaries.FindByPredicate([](const FDungeonBoundary& E){return E.Mandatory;});if(Gate){Face(Gate->A,Gate->B);M->Interact();TestFalse(TEXT("Missing key leaves lock closed"),M->CanCross(0,Gate->A,Gate->B));M->State->Keys.Add(M->Floors[0].Key);M->Interact();M->TickDoors(1);TestTrue(TEXT("Matching key and completed opening permits passage"),M->CanCross(0,Gate->A,Gate->B));}
 int32 Seen=M->State->Floors[0].Seen.Num();M->Town();M->Waypoint(M->State->SelectedWaypoint);TestTrue(TEXT("Exploration persists through town"),M->State->Floors[0].Seen.Num()>=Seen);TestTrue(TEXT("Return uses shrine"),M->Tile(M->State->X,M->State->Y)=='S');
 M->State->Party[0].Bag.Empty();TestTrue(TEXT("Stack consumables"),M->AddItem(FBagItem("health",11)));TestEqual(TEXT("Three slots for eleven potions"),M->State->Party[0].Bag.Num(),3);M->State->Party[0].Bag.Empty();for(int32 I=0;I<20;++I)TestTrue(TEXT("Fill bag"),M->AddItem(FBagItem("sword")));TestFalse(TEXT("Reject overflow atomically"),M->AddItem(FBagItem("health")));TestEqual(TEXT("Bag capped at twenty"),M->State->Party[0].Bag.Num(),20);
 M->Recruit(3);TestTrue(TEXT("Loot can use companion bag"),M->AddItem(FBagItem("health")));TestTrue(TEXT("Transfer to companion"),M->Transfer(0,0,1));TestEqual(TEXT("Transfer removes source slot"),M->State->Party[0].Bag.Num(),19);
 auto Mage=Fresh(1);Mage->State->Party[0].Bag.Empty();Mage->AddItem(FBagItem("broadsword"));TestFalse(TEXT("Mage cannot equip broadsword"),Mage->Equip(0,0));Mage->AddItem(FBagItem("magestaff",1,3));TestTrue(TEXT("Mage can equip staff"),Mage->Equip(0,1));
 int32 Before=M->State->Party[0].Level;M->GiveXP(M->NextXP(Before));TestEqual(TEXT("Level rises"),M->State->Party[0].Level,Before+1);TestEqual(TEXT("Three free points"),M->State->Party[0].Points,3);TestEqual(TEXT("Shared XP yields equal levels"),M->State->Party[0].Level,M->State->Party[1].Level);
 M->Recruit(5);M->Recruit(4);M->Recruit(6);M->Recruit(3);TestEqual(TEXT("No duplicate recruits, max five"),M->State->Party.Num(),5);TestEqual(TEXT("New recruit level matches"),M->State->Party.Last().Level,M->State->Party[0].Level);TestEqual(TEXT("No retroactive point backlog"),M->State->Party.Last().Points,0);
 M->State->Party[0].Level=98;M->State->Party[0].XP=0;M->GiveXP(100000000);TestEqual(TEXT("Level cap 99"),M->State->Party[0].Level,99);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonAccessories,"Dungeon.Inventory.AccessorySlots",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonAccessories::RunTest(const FString& Parameters){
 auto M=Fresh();auto& H=M->State->Party[0];H.Bag={FBagItem("ring",1,2),FBagItem("ring",1,3),FBagItem("charm",1,4)};
 TestTrue(TEXT("Ring equips into accessory 1"),M->Equip(0,0,1));
 TestTrue(TEXT("Second ring equips into accessory 2"),M->Equip(0,0,2));
 TestEqual(TEXT("First ring remains unchanged"),H.Gear[6].Quality,2);TestEqual(TEXT("Second ring keeps its quality"),H.Gear[7].Quality,3);
 while(H.Bag.Num()<20)H.Bag.Add(FBagItem("sword"));
 TestTrue(TEXT("Charm swaps into accessory 1 even with full bag"),M->Equip(0,0,1));
 TestEqual(TEXT("Chosen slot receives charm"),H.Gear[6].Id,FString("charm"));TestEqual(TEXT("Other slot remains ring"),H.Gear[7].Quality,3);
 TestEqual(TEXT("Replaced ring returns to original bag position"),H.Bag[0].Id,FString("ring"));TestEqual(TEXT("Returned ring quality preserved"),H.Bag[0].Quality,2);TestEqual(TEXT("No bag overflow or lost item"),H.Bag.Num(),20);
 TestFalse(TEXT("Non-accessory cannot occupy accessory slot"),M->Equip(0,1,2));TestFalse(TEXT("Invalid accessory destination rejected"),M->Equip(0,0,3));
 M->Combat=true;TestFalse(TEXT("Cannot change accessories in combat"),M->Equip(0,0,2));M->Combat=false;
 FString Prefix="AccessoryTest_"+FGuid::NewGuid().ToString(EGuidFormats::Digits)+"_";M->SavePrefix=Prefix;M->Testing=false;
 TestTrue(TEXT("Accessory selection saves"),M->Save("Roundtrip"));TestTrue(TEXT("Accessory selection reloads"),M->Load("Roundtrip"));
 TestEqual(TEXT("Slot 1 survives reload"),M->State->Party[0].Gear[6].Id,FString("charm"));TestEqual(TEXT("Slot 2 survives reload"),M->State->Party[0].Gear[7].Quality,3);
 UGameplayStatics::DeleteGameInSlot(Prefix+"Roundtrip",0);M->Testing=true;return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonBattle,"Dungeon.Core.CombatAndDeath",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonBattle::RunTest(const FString& Parameters){
 for(int32 Class=0;Class<3;++Class){auto M=Fresh(Class);M->StartCombat(109,false);TestTrue(TEXT("Combat locks movement"),M->Combat);TestFalse(TEXT("No saving during battle"),M->Save("Test"));TestFalse(TEXT("No movement in battle"),M->Move(1));Fight(M);TestFalse(TEXT("Battle resolves"),M->Combat);TestTrue(TEXT("Starting hero wins first encounter"),M->State->Party[0].HP>0);TestTrue(TEXT("Reward XP"),M->State->Party[0].XP>0);}
 for(int32 Class=0;Class<7;++Class){for(int32 Skill=0;Skill<3;++Skill){auto M=Fresh();M->State->Party.Empty();M->State->Recruited.Empty();M->Recruit(Class);auto& H=M->State->Party[0];H.Level=10;H.MP=M->MaxMP(H);H.Stats[1]=100;M->Recruit(Class==3?0:3);M->State->Party[1].HP=Skill==2&&Class==3?0:1;M->StartCombat(109,false);int32 OldMP=M->State->Party[0].MP;M->Action("Skill",Skill);TestTrue(TEXT("Every available skill spends MP"),M->State->Party[0].MP<OldMP);}}
 for(int32 PartySize=1;PartySize<=5;++PartySize)for(int32 Count=1;Count<=6;++Count){auto M=Fresh();const int RecruitIds[]={3,5,4,6};for(int32 I=1;I<PartySize;++I)M->Recruit(RecruitIds[I-1]);M->StartCombat(109,false);auto E=M->Enemies[0];M->Enemies.Init(E,Count);for(auto& H:M->State->Party){H.Stats[1]=100;H.Stats[0]=70;H.HP=M->MaxHP(H);}M->BuildOrder();M->AdvanceTurn();Fight(M);TestFalse(TEXT("All enemy/party sizes resolve"),M->Combat);}
 auto M=Fresh();M->StartCombat(175,true);TestTrue(TEXT("Boss fight starts"),M->Combat);M->Action("Flee");TestTrue(TEXT("Boss cannot be fled"),M->Combat);
 auto Normal=Fresh();Normal->Recruit(3);Normal->State->Party[1].HP=0;Normal->Town();Normal->State->Gold=1000;Normal->TownService("resurrect");TestTrue(TEXT("Normal resurrection"),Normal->State->Party[1].HP>0);
 auto HC=Fresh(0,true);HC->Recruit(3);HC->State->Party[1].HP=0;HC->Town();HC->State->Gold=1000;HC->TownService("resurrect");TestEqual(TEXT("No Hardcore resurrection"),HC->State->Party[1].HP,0);HC->State->Party[0].HP=0;HC->Defeat();TestTrue(TEXT("Hardcore run ends on protagonist death"),HC->State->Dead);
 auto Wipe=Fresh();Wipe->State->Gold=1000;Wipe->State->Keys.Add("Rusted Key");Wipe->State->Party[0].HP=0;Wipe->Defeat();TestTrue(TEXT("Paid wipe returns to town"),Wipe->State->InTown);TestTrue(TEXT("Single recovery bundle"),Wipe->State->Corpse.Num()>0);TestTrue(TEXT("Keys survive wipe"),Wipe->State->Keys.Contains("Rusted Key"));TestTrue(TEXT("Gold never in corpse"),Wipe->State->Gold>0);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonPersistence,"Dungeon.Core.PersistenceAndHunts",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonPersistence::RunTest(const FString& Parameters){
 auto M=Fresh();FString Slot="Validation_"+FGuid::NewGuid().ToString(EGuidFormats::Digits);M->Recruit(3);M->State->Keys.Add("Crypt Key");M->State->Bosses.Add(0);M->State->Postgame=true;M->State->Gold=731;M->Testing=false;
 TestTrue(TEXT("SaveGame written"),M->Save(Slot));M->State->Gold=1;M->State->Party.Empty();TestTrue(TEXT("Versioned SaveGame loads"),M->Load(Slot));TestEqual(TEXT("Gold roundtrip"),M->State->Gold,731);TestEqual(TEXT("Recruited party roundtrip"),M->State->Party.Num(),2);TestTrue(TEXT("Key/boss/postgame roundtrip"),M->State->Postgame&&M->State->Keys.Contains("Crypt Key")&&M->State->Bosses.Contains(0));UGameplayStatics::DeleteGameInSlot("Lonemoore_"+Slot,0);M->Testing=true;
 M->State->Hunts.Empty();M->GenerateOffers();int32 First=M->State->Hunts[0].Serial;M->GenerateOffers();TestEqual(TEXT("Offers persist"),M->State->Hunts[0].Serial,First);
 for(int32 I=0;I<5;++I){int32 Offer=M->State->Hunts.IndexOfByPredicate([](const FHuntRecord& H){return !H.Active;});M->AcceptHunt(Offer);}int32 Sixth=M->State->Hunts.IndexOfByPredicate([](const FHuntRecord& H){return !H.Active;});M->AcceptHunt(Sixth);int32 Active=0;for(auto H:M->State->Hunts)if(H.Active)Active++;TestEqual(TEXT("Five-hunt cap"),Active,5);
 auto& H=M->State->Hunts[0];H.Progress=H.Goal;M->Town();int32 GoldBefore=M->State->Gold;M->ClaimHunt(0);TestTrue(TEXT("Turn-in pays reward"),M->State->Gold>GoldBefore);TestTrue(TEXT("Replacement offer remains"),M->State->Hunts.ContainsByPredicate([](const FHuntRecord& R){return !R.Active;}));
 return true;
}
namespace {
int CampaignTownTrips=0,CampaignRestGold=0,CampaignPotions=0;
int Carried(UDungeonModel* M,int Hero,const FString& Id){int Count=0;for(const auto& B:M->State->Party[Hero].Bag)if(B.Id==Id)Count+=B.Count;return Count;}
bool CampaignTownSupplies(UDungeonModel* M){
 if(!M->State->InTown||M->Combat)return false;++CampaignTownTrips;
 // Use the merchant exactly as the player does. Unequipped loot otherwise fills
 // the old bot's bags, causing every subsequent potion drop to be rejected.
 for(int H=0;H<M->State->Party.Num();++H)for(int Slot=M->State->Party[H].Bag.Num()-1;Slot>=0;--Slot){auto D=M->Item(M->State->Party[H].Bag[Slot].Id);if(D&&!D->Slot.IsEmpty())M->Sell(H,Slot);}
 for(const FString& Service:TArray<FString>{TEXT("resurrect"),TEXT("rest")})if(M->NeedsCare(Service)){const int Before=M->State->Gold;M->TownService(Service);CampaignRestGold+=Before-M->State->Gold;}
 if(M->NeedsCare(TEXT("rest"))||M->NeedsCare(TEXT("resurrect")))return false;
 for(int H=M->State->Party.Num()-1;H>=0;--H)for(const FString& Id:TArray<FString>{TEXT("health"),TEXT("mana")}){
  for(int Attempt=0;Attempt<8&&Carried(M,H,Id)<3;++Attempt){int Before=Carried(M,H,Id);
   if(H>0){int Slot=M->State->Party[0].Bag.IndexOfByPredicate([&](const FBagItem& B){return B.Id==Id;});if(Slot>=0)M->Transfer(0,Slot,H);}
   if(Carried(M,H,Id)>=3)break;M->Buy(Id);
   if(H>0){int Slot=M->State->Party[0].Bag.IndexOfByPredicate([&](const FBagItem& B){return B.Id==Id;});if(Slot>=0)M->Transfer(0,Slot,H);}
   if(Carried(M,H,Id)==Before)break;
  }
 }
 return true;
}
void CampaignUseSupplies(UDungeonModel* M){
 for(int H=0;H<M->State->Party.Num();++H)for(int Attempt=0;Attempt<12&&M->State->Party[H].HP>0;++Attempt){
  const bool Health=M->State->Party[H].HP<M->MaxHP(M->State->Party[H])*.82f;
  const bool Mana=M->State->Party[H].MP<M->MaxMP(M->State->Party[H])*.40f;if(!Health&&!Mana)break;
  bool Used=false;for(int Owner=0;Owner<M->State->Party.Num()&&!Used;++Owner){int Slot=M->State->Party[Owner].Bag.IndexOfByPredicate([&](const FBagItem& B){return Health?(B.Id==TEXT("health")||B.Id==TEXT("greater_health")):(B.Id==TEXT("mana")||B.Id==TEXT("greater_mana"));});if(Slot>=0){Used=M->UseItem(Owner,Slot,H);CampaignPotions+=Used;}}
  if(!Used)break;
 }
}
void CampaignFight(UDungeonModel* M){
 for(int32 I=0;I<800&&M->Combat;++I){
  if(!M->State->Party.IsValidIndex(M->Acting))break;auto H=M->State->Party[M->Acting];
  const bool CanCast=!H.Status.Contains("Silence");
  bool Wounded=M->State->Party.ContainsByPredicate([&](const FPartyHero& A){return A.HP>0&&A.HP<M->MaxHP(A)*.55f;});
  bool NeedsGuard=M->BossFight&&!M->Betrayal&&M->State->Party.ContainsByPredicate([](const FPartyHero& A){return A.HP>0&&A.Status.FindRef("Guard")<=1;});
  if(CanCast&&NeedsGuard&&H.Class==5&&H.Level>=5&&H.MP>=M->Skill(M->Classes[H.Class].Skills[1])->Cost)M->Action("Skill",1);
  else if(CanCast&&NeedsGuard&&H.Class==0&&H.Level>=10&&H.MP>=M->Skill(M->Classes[H.Class].Skills[2])->Cost)M->Action("Skill",2);
  else if(CanCast&&H.Class==3&&H.MP>=4&&Wounded)M->Action("Skill",0);
  else if(CanCast&&H.Class==5&&H.Level>=10&&H.MP>=10&&Wounded)M->Action("Skill",2);
  else if(H.HP<M->MaxHP(H)/3&&H.Bag.ContainsByPredicate([](const FBagItem& B){return B.Id=="health"||B.Id=="greater_health";})){M->Action("Item");++CampaignPotions;}
  else if(H.MP<M->Skill(M->Classes[H.Class].Skills[0])->Cost&&H.Bag.ContainsByPredicate([](const FBagItem& B){return B.Id=="mana"||B.Id=="greater_mana";})){M->Action("Mana");++CampaignPotions;}
  else if(H.Class==3)M->Action("Attack");
  else if(CanCast&&H.MP>=M->Skill(M->Classes[H.Class].Skills[0])->Cost)M->Action("Skill",0);
  else M->Action("Attack");
 }
}
bool Travel(UDungeonModel* M,int32 Destination,bool AllowRest=true,bool ClearedOnly=false){
 auto S=M->State;auto& D=M->Floors[S->Floor];int32 W=D.Rows[0].Len();int32 Start=M->Cell(S->X,S->Y);TMap<int32,int32> Parent;TArray<int32> Queue;Queue.Add(Start);Parent.Add(Start,-1);const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
 for(int32 Q=0;Q<Queue.Num();++Q){int32 C=Queue[Q];if(C==Destination)break;for(int32 Dir=0;Dir<4;++Dir){int32 X=C%W+DX[Dir],Y=C/W+DY[Dir],N=M->Cell(X,Y);TCHAR T=M->Tile(X,Y);const auto* Edge=M->Boundary(S->Floor,C,N);if(T=='#'||T=='~'||Parent.Contains(N)||!Edge||Edge->Kind=="Wall"||Edge->Kind=="Pit")continue;if(ClearedOnly&&(!S->Floors[S->Floor].Seen.Contains(N)||((T=='E'||T=='B'||T=='m')&&!M->IsDefeated(N))||(T=='H'&&M->HuntAt(N)>=0)))continue;if(!M->CanCross(S->Floor,C,N)&&!M->HasRequirement(S->Floor,*Edge))continue;Parent.Add(N,C);Queue.Add(N);}}
 if(!Parent.Contains(Destination))return false;TArray<int32> Path;for(int32 C=Destination;C!=Start;C=Parent[C])Path.Add(C);
 for(int32 I=Path.Num()-1;I>=0;--I){int32 C=Path[I],X=C%W,Y=C/W;TCHAR T=M->Tile(X,Y);
  const bool Encounter=((T=='E'||T=='B'||T=='m')&&!M->IsDefeated(C))||(T=='H'&&M->HuntAt(C)>=0);
  if(AllowRest&&Encounter){CampaignUseSupplies(M);
   const bool NeedsRest=S->Party.ContainsByPredicate([&](const FPartyHero& H){return H.HP<M->MaxHP(H)*.75f||H.MP<M->MaxMP(H)*.25f;});
   if(NeedsRest){const int Checkpoint=M->Cell(S->X,S->Y),Waypoint=S->SelectedWaypoint;M->Town();if(!CampaignTownSupplies(M))return false;M->Waypoint(Waypoint);
    // No teleport back to the encounter: retrace a legal route from the last
    // activated shrine, with every previously undefeated enemy still blocking.
    if(!Travel(M,Checkpoint,false,true))return false;
   }
  }
  int32 Dir=X>S->X?1:X<S->X?3:Y>S->Y?2:0;while(S->Facing!=Dir)M->Rotate(1);if(!M->CanCross(S->Floor,M->Cell(S->X,S->Y),C)){M->Interact();M->TickDoors(1);}if(T=='T')M->Interact();if(!M->Move(1)){if(M->Combat){CampaignFight(M);if(M->Screen!="Dungeon"||!M->Move(1))return false;}else return false;}
  if(T=='K'||T=='C'||T=='$'||T=='!'||T=='L'||T=='V'){M->Interact();M->TickDoors(1);}
  for(int32 H=0;H<S->Party.Num();++H){auto& Hero=S->Party[H];while(Hero.Points>0&&Hero.HP>0)M->Allocate(H,M->Classes[Hero.Class].Primary);if(Hero.HP>0&&Hero.HP<M->MaxHP(Hero)/2){int32 Potion=Hero.Bag.IndexOfByPredicate([](const FBagItem& B){return B.Id=="health"||B.Id=="greater_health";});if(Potion>=0)CampaignPotions+=M->UseItem(H,Potion,H);}}
 }
 return M->Cell(S->X,S->Y)==Destination;
}
int32 FindTile(UDungeonModel* M,TCHAR T){auto& R=M->Floors[M->State->Floor].Rows;for(int32 Y=0;Y<R.Num();++Y)for(int32 X=0;X<R[Y].Len();++X)if(R[Y][X]==T)return M->Cell(X,Y);return -1;}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonCampaign,"Dungeon.Story.CompleteRoutesAndEndings",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonCampaign::RunTest(const FString& Parameters){
 FString Report;
 // EncounterRoster also derives its random stream from RunId. Pin both streams
 // so a route regression is reproducible instead of changing with each GUID.
 for(int32 StartingClass=0;StartingClass<7;++StartingClass){CampaignTownTrips=CampaignRestGold=CampaignPotions=0;auto M=Fresh(StartingClass);M->State->RunId=TEXT("DungeonCampaignRegression73519");bool OK=true;
  for(int32 F=0;F<M->Floors.Num()&&OK;++F){
   TestEqual(TEXT("Sequential floor progression"),M->State->Floor,F);auto& D=M->Floors[F];
   TArray<int> Goals;for(TCHAR T:TArray<TCHAR>{'R','K','L','C','$'}){int Target=FindTile(M,T);if(Target>=0)Goals.Add(Target);}
   int Width=D.Rows[0].Len();for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<Width;++X)if(D.Rows[Y][X]=='E')Goals.Add(Y*Width+X);
   for(TCHAR T:TArray<TCHAR>{'B','V'}){int Target=FindTile(M,T);if(Target>=0)Goals.Add(Target);}
   for(int Target:Goals){TCHAR T=D.Rows[Target/Width][Target%Width];
    if(M->State->Party.ContainsByPredicate([&](const FPartyHero& Hero){return Hero.HP<M->MaxHP(Hero)*.75f||Hero.MP<M->MaxMP(Hero)*.25f;})){M->Town();if(!CampaignTownSupplies(M)){AddError(TEXT("Expedition cannot afford or receive ordinary recovery services"));OK=false;break;}M->Waypoint(M->State->SelectedWaypoint);}
    if(!Travel(M,Target)){AddError(FString::Printf(TEXT("%s cannot reach %c in floor %d: %s | %s"),*M->Classes[StartingClass].Name,T,F,*D.Name,*M->Notice));OK=false;break;}if(T=='V')M->Interact();}
   Report+=FString::Printf(TEXT("%s | floor %d | level %d | party %d | gold %d | town_trips %d | care_gold %d | potions %d | %s\n"),*M->Classes[StartingClass].Name,F,M->State->Party[0].Level,M->State->Party.Num(),M->State->Gold,CampaignTownTrips,CampaignRestGold,CampaignPotions,OK?TEXT("PASS"):TEXT("FAIL"));
   if(!OK)break;
   if(F+1<M->Floors.Num()){
    M->Town();if(!CampaignTownSupplies(M)){AddError(TEXT("Floor-end resupply failed using the ordinary economy"));OK=false;break;}
    M->Waypoint(M->State->SelectedWaypoint);int32 Exit=FindTile(M,'>');OK=Travel(M,Exit);TestTrue(TEXT("Reach the descent"),OK);if(OK)M->Interact();
   }
  }
  if(!OK)continue;TestEqual(TEXT("All guaranteed recruits"),M->State->Party.Num(),5);TestTrue(TEXT("All four recruited ids"),M->State->Recruited.Contains(3)&&M->State->Recruited.Contains(4)&&M->State->Recruited.Contains(5)&&M->State->Recruited.Contains(6));
  // Prepare through ordinary town services before the final fight; the former harness
  // skipped its final rest and never upgraded equipment after the Phase 2 boss changes.
  M->Town();M->TownService("resurrect");M->TownService("rest");
  for(int H=0;H<M->State->Party.Num();++H)for(int Q=0;Q<3;++Q){M->Upgrade(H,0);M->Upgrade(H,3);}
  M->Waypoint(M->State->SelectedWaypoint);
  TestTrue(TEXT("Reach Astra"),Travel(M,FindTile(M,'A')));M->Interact();TestEqual(TEXT("Final choice appears"),M->Screen,FString("Choice"));
  auto Before=DuplicateObject<UDungeonSave>(M->State,M);M->FinalChoice(false);CampaignFight(M);TestEqual(TEXT("Astra defeated, good ending"),M->State->Ending,1);M->FinishCredits();M->BeginPostgame();TestTrue(TEXT("Hell Hunts unlocked only after stinger"),M->State->Postgame);TestTrue(TEXT("All new offers are Hell Hunts"),!M->State->Hunts.ContainsByPredicate([](const FHuntRecord& H){return !H.Hell;}));
  M->State=DuplicateObject<UDungeonSave>(Before,M);M->Screen="Choice";M->FinalChoice(true);CampaignFight(M);TestEqual(TEXT("Betrayal fight reaches dark ending"),M->State->Ending,2);M->FinishCredits();M->BeginPostgame();TestFalse(TEXT("Dark ending cannot unlock Hell Hunts"),M->State->Postgame);
 }
 FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("Validation/campaign_routes.txt")));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonQuarry,"Dungeon.Core.QuarriesAndEndingPersistence",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonQuarry::RunTest(const FString& Parameters){
 auto M=Fresh();int32 Hunt=M->State->Hunts.IndexOfByPredicate([](const FHuntRecord& H){return H.Kind==3;});TestTrue(TEXT("Named quarry offered"),Hunt>=0);if(Hunt<0)return false;M->AcceptHunt(Hunt);int32 Cell=FindTile(M,'H');TestTrue(TEXT("Fixed quarry alcove exists"),Cell>=0);TestEqual(TEXT("Accepted quarry becomes visible"),M->HuntAt(Cell),Hunt);
 TestTrue(TEXT("Quarry reached and defeated through movement/combat"),Travel(M,Cell));TestEqual(TEXT("Quarry marks its hunt complete"),M->State->Hunts[Hunt].Progress,1);TestEqual(TEXT("Completed quarry does not immediately respawn"),M->HuntAt(Cell),-1);TestFalse(TEXT("Optional quarry does not break main boss seal"),M->State->Bosses.Contains(0));
 auto SaveTest=Fresh();SaveTest->State->Ending=1;SaveTest->Screen="Ending";SaveTest->Testing=false;FString Slot="EndingValidation_"+FGuid::NewGuid().ToString(EGuidFormats::Digits);TestTrue(TEXT("Victory save written"),SaveTest->Save(Slot));TestTrue(TEXT("Victory save restored"),SaveTest->Load(Slot));TestEqual(TEXT("Victory restore resumes ending flow"),SaveTest->Screen,FString("Ending"));UGameplayStatics::DeleteGameInSlot("Lonemoore_"+Slot,0);
 return true;
}
#endif
