#include "DungeonCombatAudio.h"
#include "DungeonModel.h"

namespace {
struct FSkillSound { const TCHAR* Name; const TCHAR* Cue; };
const FSkillSound SkillSounds[] = {
 {TEXT("Power Strike"),TEXT("power_strike")}, {TEXT("Cleave"),TEXT("cleave")}, {TEXT("Shield Wall"),TEXT("shield_wall")},
 {TEXT("Fireball"),TEXT("fireball")}, {TEXT("Ice Lance"),TEXT("ice_lance")}, {TEXT("Chain Lightning"),TEXT("chain_lightning")},
 {TEXT("Power Shot"),TEXT("power_shot")}, {TEXT("Multi-Shot"),TEXT("multi_shot")}, {TEXT("Hunter's Mark"),TEXT("hunters_mark")},
 {TEXT("Heal"),TEXT("heal")}, {TEXT("Holy Smite"),TEXT("holy_smite")}, {TEXT("Resurrection"),TEXT("resurrection")},
 {TEXT("Backstab"),TEXT("backstab")}, {TEXT("Poison Blade"),TEXT("poison_blade")}, {TEXT("Small Grenade"),TEXT("grenade")},
 {TEXT("Holy Strike"),TEXT("holy_strike")}, {TEXT("Divine Shield"),TEXT("divine_shield")}, {TEXT("Lay on Hands"),TEXT("lay_on_hands")},
 {TEXT("Shadow Bolt"),TEXT("shadow_bolt")}, {TEXT("Curse"),TEXT("curse")}, {TEXT("Soul Drain"),TEXT("soul_drain")}
};
}

const TArray<FString>& DungeonCombatAudio::CueIds() {
 static const TArray<FString> Ids=[] {
  TArray<FString> Result={TEXT("hit"),TEXT("critical"),TEXT("miss"),TEXT("guard"),TEXT("defend"),TEXT("dodge"),TEXT("item"),TEXT("mana"),TEXT("escape"),TEXT("escape_fail")};
  for(const auto& Sound:SkillSounds)Result.Add(Sound.Cue);
  Result.Add(TEXT("arcane_bolt"));Result.Add(TEXT("status_bind"));
  return Result;
 }();
 return Ids;
}

FString DungeonCombatAudio::ElementCue(const FString& Type) {
 if(Type==TEXT("Physical"))return TEXT("hit");
 if(Type==TEXT("Fire"))return TEXT("fireball");
 if(Type==TEXT("Ice"))return TEXT("ice_lance");
 if(Type==TEXT("Lightning"))return TEXT("chain_lightning");
 if(Type==TEXT("Holy"))return TEXT("holy_smite");
 if(Type==TEXT("Dark"))return TEXT("shadow_bolt");
 if(Type==TEXT("Poison"))return TEXT("poison_blade");
 return TEXT("arcane_bolt");
}

FString DungeonCombatAudio::SkillCue(const FSkillDef& Skill) {
 // Authored skill identity distinguishes related abilities within each class.
 // Semantic fallbacks also support new content without class-number routing.
 for(const auto& Sound:SkillSounds)if(Skill.Name==Sound.Name)return Sound.Cue;
 if(Skill.Target==TEXT("revive"))return TEXT("resurrection");
 if(Skill.Target==TEXT("heal"))return TEXT("heal");
 if(Skill.Effect==TEXT("Drain"))return TEXT("soul_drain");
 if(Skill.Effect==TEXT("Curse"))return TEXT("curse");
 if(Skill.Effect==TEXT("Mark"))return TEXT("hunters_mark");
 if(Skill.Effect==TEXT("Poison"))return TEXT("poison_blade");
 if(Skill.Effect==TEXT("Guard"))return Skill.Type==TEXT("Holy")?TEXT("divine_shield"):TEXT("shield_wall");
 if(Skill.Type==TEXT("Physical")&&!Skill.Effect.IsEmpty())return TEXT("status_bind");
 return ElementCue(Skill.Type);
}
