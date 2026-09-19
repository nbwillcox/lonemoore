#pragma once
#include "CoreMinimal.h"

struct FGeometry;
class FSlateWindowElementList;

// Original 96px, limited-palette sprites, animated in eight stepped frames.
// Center and Size use the caller's Slate geometry. Progress is normalized 0..1.
namespace DungeonCombatVFX {
 float Duration(const FString& Cue);
 int32 Paint(const FString& Cue,const FGeometry& Geometry,FSlateWindowElementList& Elements,int32 Layer,FVector2D Center,float Size,float Progress);
}
