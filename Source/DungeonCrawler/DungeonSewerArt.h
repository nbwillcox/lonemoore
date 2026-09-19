#pragma once
#include "CoreMinimal.h"
class ADungeonController; class AActor;
namespace SewerArt {
FString MaterialFor(const FString& MeshName,const FString& OriginalMaterial);
void Decorate(ADungeonController* Host,AActor* Root);
}
