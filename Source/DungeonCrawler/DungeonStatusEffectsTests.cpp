#include "DungeonModel.h"
#include "DungeonStatusEffects.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonStatuses,"Dungeon.Combat.StatusEffects",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonStatuses::RunTest(const FString& Parameters) {
 TMap<FString,int32> Status;int32 HP=100;
 DungeonStatusEffects::Apply(Status,"Poison");DungeonStatusEffects::Apply(Status,"Poison");
 TestEqual(TEXT("Reapplication refreshes rather than stacking poison"),Status.FindRef("Poison"),3);
 for(int32 I=0;I<3;++I)DungeonStatusEffects::Tick(Status,HP,1);
 TestEqual(TEXT("Poison damages on all three ticks"),HP,88);TestTrue(TEXT("Poison expires"),Status.IsEmpty());
 DungeonStatusEffects::Apply(Status,"Stun");TestTrue(TEXT("Stun skips one turn"),DungeonStatusEffects::Tick(Status,HP,1));TestFalse(TEXT("Stun cannot lock a second turn"),DungeonStatusEffects::Tick(Status,HP,1));
 DungeonStatusEffects::Apply(Status,"Plague");TestEqual(TEXT("Plague halves healing"),DungeonStatusEffects::Healing(Status,20),10);
 Status.Add("Guard",2);DungeonStatusEffects::Cleanse(Status);TestFalse(TEXT("Tonic removes illness"),Status.Contains("Plague"));TestEqual(TEXT("Tonic preserves beneficial guard"),Status.FindRef("Guard"),2);
 Status.Empty();Status.Add("Weakness",3);TestTrue(TEXT("Weakness reduces outgoing damage"),FMath::IsNearlyEqual(DungeonStatusEffects::DamageScale(Status),.8f));
 Status.Add("Curse",3);TestTrue(TEXT("Curse and weakness combine predictably"),FMath::IsNearlyEqual(DungeonStatusEffects::DamageScale(Status),.56f));
 Status.Empty();Status.Add("Bleed",1);Status.Add("Burn",1);HP=2;DungeonStatusEffects::Tick(Status,HP,10);TestEqual(TEXT("Stacked damage never makes health negative"),HP,0);
 auto M=NewObject<UDungeonModel>();TestTrue(TEXT("Campaign initializes"),M->Initialize());M->Testing=true;M->NewGame(0,false);
 auto Has=[&](const FString& Id,const FString& Key){const auto* E=M->EnemyDefs.FindByPredicate([&](const FEnemyDef& D){return D.Id==Id;});return E&&E->Status==Key;};
 TestTrue(TEXT("Fungal rats inflict plague"),Has("fungal","Plague"));TestTrue(TEXT("Slimes slow"),Has("slime","Slow"));TestTrue(TEXT("Cursed rats weaken"),Has("rat","Weakness"));TestTrue(TEXT("Shamans silence"),Has("shaman","Silence"));TestTrue(TEXT("Spiders retain poison"),Has("spider","Poison"));
 auto& H=M->State->Party[0];H.HP=1;H.Status.Add("Plague",3);H.Status.Add("Guard",2);H.Bag={FBagItem("remedy")};
 TestTrue(TEXT("Existing cleansing tonic remains usable"),M->UseItem(0,0,0));TestFalse(TEXT("Tonic clears new status"),H.Status.Contains("Plague"));TestTrue(TEXT("Tonic keeps shield"),H.Status.Contains("Guard"));TestEqual(TEXT("Tonic consumes one item"),H.Bag.Num(),0);
 M->EnterFloor(0);M->Combat=true;M->Acting=0;M->Target=0;M->TimedCombat=false;M->TurnDelay=0;M->PendingEnemy=-1;
 FEnemyUnit E;E.HP=E.MaxHP=100;E.Speed=10;M->Enemies={E};H.HP=M->MaxHP(H);H.MP=M->MaxMP(H);H.Status.Empty();H.Status.Add("Silence",3);
 const int32 Mana=H.MP;M->Action("Skill",0);TestEqual(TEXT("Silence blocks skill without spending mana"),H.MP,Mana);TestEqual(TEXT("Blocked skill keeps actor available"),M->Acting,0);
 H.Stats[1]=15;H.Status.Add("Slow",3);M->BuildOrder();TestFalse(TEXT("Slow drops faster hero below enemy initiative"),M->Order[0].Hero);
 H.Status.Remove("Slow");M->BuildOrder();TestTrue(TEXT("Recovered hero regains faster initiative"),M->Order[0].Hero);
 return true;
}
#endif
