#include "DungeonModel.h"
#include "DungeonCombatVFX.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonCombatFXRouting,"Dungeon.Audio.VisualTargetRouting",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonCombatFXRouting::RunTest(const FString& Parameters){
 auto M=NewObject<UDungeonModel>();M->Testing=true;M->Initialize();M->State->Floor=0;M->State->Floors.SetNum(M->Floors.Num());M->Recruit(1);M->Recruit(3);
 auto Ready=[&](int32 Class){
  M->Combat=true;M->TimedCombat=true;M->Acting=0;M->Target=1;M->TurnDelay=0;M->PendingEnemy=-1;M->CombatEffects.Empty();M->CombatCues.Empty();M->State->Party[0].Class=Class;
  for(auto& H:M->State->Party){H.Level=20;H.HP=M->MaxHP(H);H.MP=999;H.Status.Empty();}
  FEnemyUnit E;E.HP=E.MaxHP=100000;E.Attack=1;E.Type="Physical";M->Enemies.Init(E,3);
 };
 Ready(1);M->Action("Skill",0);
 TestEqual(TEXT("One fireball sprite for a single selected enemy"),M->CombatEffects.Num(),1);
 if(M->CombatEffects.Num()==1){const auto& FX=M->CombatEffects[0];TestEqual(TEXT("Fireball art matches audio"),FX.Cue,M->CombatCues[0]);TestEqual(TEXT("Selected enemy receives effect"),FX.Target,1);TestFalse(TEXT("Offense never paints on hero"),FX.Hero);TestTrue(TEXT("Turn pause covers complete animation"),M->TurnDelay>=DungeonCombatVFX::Duration(FX.Cue));}
 Ready(1);M->Enemies[0].HP=0;M->Action("Skill",2);
 TestEqual(TEXT("Area spell paints each living target only"),M->CombatEffects.Num(),2);TestEqual(TEXT("Area sound plays only once"),M->CombatCues.Num(),1);
 Ready(3);M->State->Party[1].HP=1;M->Action("Skill",0);
 TestEqual(TEXT("Heal emits one sprite"),M->CombatEffects.Num(),1);if(M->CombatEffects.Num()==1){TestTrue(TEXT("Healing paints on party"),M->CombatEffects[0].Hero);TestEqual(TEXT("Healing follows actual lowest-health recipient"),M->CombatEffects[0].Target,1);}
 Ready(0);M->Action("Defend");TestEqual(TEXT("Defend cue is distinct"),M->CombatCues[0],FString("defend"));TestTrue(TEXT("Defend paints on acting hero"),M->CombatEffects[0].Hero);
 Ready(1);M->State->Party[0].MP=0;M->Action("Skill",0);TestTrue(TEXT("Rejected cast emits no sprite"),M->CombatEffects.IsEmpty());TestTrue(TEXT("Rejected cast emits no sound"),M->CombatCues.IsEmpty());
 Ready(0);M->State->Party[0].Status.Add("Blind",2);bool SawMiss=false;
 for(int32 I=0;I<1024&&!SawMiss;++I){M->Acting=0;M->TurnDelay=0;M->CombatEffects.Empty();M->CombatCues.Empty();int32 Before=M->Enemies[1].HP;M->Action("Attack");if(M->CombatCues.Contains("miss")){SawMiss=true;TestEqual(TEXT("Miss does no damage"),M->Enemies[1].HP,Before);TestEqual(TEXT("Miss paints an evasion effect, never slash impact"),M->CombatEffects[0].Cue,FString("miss"));}}
 TestTrue(TEXT("Miss branch exercised"),SawMiss);
 Ready(0);M->CombatFeedback("critical",1);M->Combat=false;M->TickCombat(2.f);TestTrue(TEXT("Effects expire even after combat ends"),M->CombatEffects.IsEmpty());
 Ready(0);M->Enemies[1].HP=1;M->State->Party[0].Status.Empty();
 for(int32 I=0;I<1024&&M->Enemies[1].HP>0;++I){M->Acting=0;M->TurnDelay=0;M->CombatEffects.Empty();M->Action("Attack");}
 TestTrue(TEXT("Lethal strike retains its target until presentation ends"),M->Enemies.IsValidIndex(1)&&M->Enemies[1].HP<=0&&!M->CombatEffects.IsEmpty()&&M->Combat);
 return true;
}
#endif
