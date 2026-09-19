#include "DungeonCombatAudio.h"
#include "DungeonModel.h"
#include "Misc/AutomationTest.h"
#include "Sound/SoundWave.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace {
UDungeonModel* AudioModel(int32 Class) {
 auto M=NewObject<UDungeonModel>();M->Testing=true;M->Initialize();
 M->State->Floor=0;M->State->Floors.SetNum(M->Floors.Num());
 M->Recruit(Class);M->Recruit(Class==3?0:3);
 return M;
}
void ReadyAudioAction(UDungeonModel* M,bool FallenCompanion=false) {
 M->Combat=true;M->TimedCombat=true;M->Acting=0;M->Target=0;M->Turn=0;
 M->TurnDelay=0;M->PendingEnemy=-1;M->CombatCues.Empty();M->CombatEffects.Empty();
 for(auto& Hero:M->State->Party){Hero.Level=10;Hero.HP=M->MaxHP(Hero);Hero.MP=999;Hero.Status.Empty();}
 M->State->Party[1].HP=FallenCompanion?0:1;
 FEnemyUnit Enemy;Enemy.Name=TEXT("Audio target");Enemy.Type=TEXT("Physical");Enemy.HP=Enemy.MaxHP=100000;Enemy.Attack=1;
 M->Enemies.Init(Enemy,3);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonAudioActions,"Dungeon.Audio.SkillFeedback",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonAudioActions::RunTest(const FString& Parameters) {
 const TCHAR* Expected[7][3]={
  {TEXT("power_strike"),TEXT("cleave"),TEXT("shield_wall")},
  {TEXT("fireball"),TEXT("ice_lance"),TEXT("chain_lightning")},
  {TEXT("power_shot"),TEXT("multi_shot"),TEXT("hunters_mark")},
  {TEXT("heal"),TEXT("holy_smite"),TEXT("resurrection")},
  {TEXT("backstab"),TEXT("poison_blade"),TEXT("grenade")},
  {TEXT("holy_strike"),TEXT("divine_shield"),TEXT("lay_on_hands")},
  {TEXT("shadow_bolt"),TEXT("curse"),TEXT("soul_drain")}
 };
 TSet<FString> Selected;
 for(int32 Class=0;Class<7;++Class){auto M=AudioModel(Class);
  for(int32 Skill=0;Skill<3;++Skill){
   ReadyAudioAction(M,Class==3&&Skill==2);const int32 BeforeMP=M->State->Party[0].MP;
   M->Action(TEXT("Skill"),Skill);
   TestTrue(TEXT("Valid skill is executed"),M->State->Party[0].MP<BeforeMP);
   TestEqual(TEXT("One effect per cast, including area and party skills"),M->CombatCues.Num(),1);
   if(M->CombatCues.Num()==1){TestEqual(M->Classes[Class].Skills[Skill],M->CombatCues[0],FString(Expected[Class][Skill]));Selected.Add(M->CombatCues[0]);}
  }
  ReadyAudioAction(M);M->State->Party[0].MP=0;M->Action(TEXT("Skill"),0);TestTrue(TEXT("Rejected insufficient-MP cast stays silent"),M->CombatCues.IsEmpty());
  ReadyAudioAction(M);M->State->Party[0].Level=1;M->Action(TEXT("Skill"),2);TestTrue(TEXT("Locked skill stays silent"),M->CombatCues.IsEmpty());
  ReadyAudioAction(M);M->Action(TEXT("Skill"),999);TestTrue(TEXT("Invalid skill stays silent"),M->CombatCues.IsEmpty());
 }
 TestEqual(TEXT("All 21 active skills have distinct authored feedback"),Selected.Num(),21);
 auto Cleric=AudioModel(3);ReadyAudioAction(Cleric);Cleric->Action(TEXT("Skill"),2);
 TestTrue(TEXT("Resurrection without a fallen ally stays silent"),Cleric->CombatCues.IsEmpty());
 ReadyAudioAction(Cleric,true);Cleric->State->Hardcore=true;Cleric->Action(TEXT("Skill"),2);
 TestTrue(TEXT("Forbidden Hardcore resurrection stays silent"),Cleric->CombatCues.IsEmpty());
 auto Warrior=AudioModel(0);ReadyAudioAction(Warrior);Warrior->Action(TEXT("Defend"));
 TestEqual(TEXT("Defend uses one guard cue"),Warrior->CombatCues.Num(),1);
 if(Warrior->CombatCues.Num()==1)TestEqual(TEXT("Defend feedback"),Warrior->CombatCues[0],FString(TEXT("defend")));
 for(int32 I=0;I<64;++I){ReadyAudioAction(Warrior);Warrior->Action(TEXT("Attack"));
  TestEqual(TEXT("Attack has one primary cue"),Warrior->CombatCues.Num(),1);
  if(Warrior->CombatCues.Num()==1)TestTrue(TEXT("Hit, critical or miss is used without generic spell audio"),Warrior->CombatCues[0]==TEXT("hit")||Warrior->CombatCues[0]==TEXT("critical")||Warrior->CombatCues[0]==TEXT("miss"));
 }
 ReadyAudioAction(Warrior);Warrior->Enemies[0].Id=TEXT("astra");Warrior->Round=2;Warrior->EnemyAction(0);
 TestEqual(TEXT("Boss party attack has one effect, not one per victim"),Warrior->CombatCues.Num(),1);
 if(Warrior->CombatCues.Num()==1)TestEqual(TEXT("Boss fire attack has fire feedback"),Warrior->CombatCues[0],FString(TEXT("fireball")));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonAudioSemantics,"Dungeon.Audio.SemanticFallbacks",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonAudioSemantics::RunTest(const FString& Parameters) {
 FSkillDef New;New.Name=TEXT("Future unnamed skill");New.Target=TEXT("enemy");New.Type=TEXT("Fire");
 TestEqual(TEXT("Future fire skills use fire"),DungeonCombatAudio::SkillCue(New),FString(TEXT("fireball")));
 New.Type=TEXT("Ice");New.Effect=TEXT("Stun");TestEqual(TEXT("Ice remains ice even when it stuns"),DungeonCombatAudio::SkillCue(New),FString(TEXT("ice_lance")));
 New.Type=TEXT("Physical");New.Effect=TEXT("Fear");TestEqual(TEXT("Physical status skill uses status feedback"),DungeonCombatAudio::SkillCue(New),FString(TEXT("status_bind")));
 New.Type=TEXT("Dark");New.Effect=TEXT("Drain");TestEqual(TEXT("Drain takes precedence over generic darkness"),DungeonCombatAudio::SkillCue(New),FString(TEXT("soul_drain")));
 New.Effect.Empty();New.Target=TEXT("heal");TestEqual(TEXT("Healing is recognized by effect target"),DungeonCombatAudio::SkillCue(New),FString(TEXT("heal")));
 New.Target=TEXT("enemy");New.Type=TEXT("Arcane");TestEqual(TEXT("Unknown magic has an explicit bolt fallback"),DungeonCombatAudio::SkillCue(New),FString(TEXT("arcane_bolt")));
 const auto& Ids=DungeonCombatAudio::CueIds();TSet<FString> Unique;for(const auto& Id:Ids)Unique.Add(Id);
 TestEqual(TEXT("Bounded preload catalogue"),Ids.Num(),33);TestEqual(TEXT("No duplicate preload assets"),Unique.Num(),Ids.Num());
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonAudioAssets,"Dungeon.Audio.ImportedEffects",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDungeonAudioAssets::RunTest(const FString& Parameters) {
 for(const auto& Id:DungeonCombatAudio::CueIds()){
  auto Sound=LoadObject<USoundWave>(nullptr,*(TEXT("/Game/Game/Audio/A_")+Id+TEXT(".A_")+Id));
  TestNotNull(TEXT("Imported combat sound: ")+Id,Sound);
  if(Sound){TestTrue(TEXT("Compact nonempty sound: ")+Id,Sound->Duration>.1f&&Sound->Duration<2.f);TestFalse(TEXT("Combat effects never loop: ")+Id,Sound->bLooping);}
 }
 return true;
}
#endif
