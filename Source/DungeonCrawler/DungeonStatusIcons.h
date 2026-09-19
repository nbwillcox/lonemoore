#pragma once

#include "CoreMinimal.h"
#include "Layout/Geometry.h"

class FSlateWindowElementList;

// Small, resolution-independent emblems. Position and size use the UI's local coordinates.
namespace DungeonStatusIcons
{
 FLinearColor Color(const FString& Status);
 void Draw(const FGeometry& Geometry,FSlateWindowElementList& Elements,int32& Layer,
           const FString& Status,FVector2D Position,float Size=22.f);
}
