#pragma once
#include "CoreMinimal.h"
class ADungeonController;
class AActor;
namespace RoomDressing {
 void AddPreloadAssets(int32 Region,TArray<FSoftObjectPath>& Assets);
 void Decorate(ADungeonController* Host,AActor* Root);
}
