#pragma once
#include "CoreMinimal.h"

namespace DungeonPresentation {
// Preserve a readable warm pool at the player, with darker distant rooms.
constexpr float PlayerTorchIntensity=30000.f;
constexpr float PlayerFillIntensity=5500.f;
constexpr float RoomFillIntensity=34000.f;
constexpr float BossApproachFillIntensity=18000.f;
constexpr float SpriteFloorInset=4.f;
inline float GroundedEnemyCenter(float Height,float BottomPadding){
 return Height*(.5f-FMath::Clamp(BottomPadding,0.f,.35f))-SpriteFloorInset;
}
}
