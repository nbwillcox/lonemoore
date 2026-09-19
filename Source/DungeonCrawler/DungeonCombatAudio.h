#pragma once

#include "CoreMinimal.h"

struct FSkillDef;

namespace DungeonCombatAudio {
// Stable asset IDs under /Game/Game/Audio/A_<id>; cache these before combat.
const TArray<FString>& CueIds();
FString SkillCue(const FSkillDef& Skill);
FString ElementCue(const FString& Type);
}
