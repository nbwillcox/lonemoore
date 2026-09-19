#pragma once
#include "CoreMinimal.h"
class ADungeonController;
class AActor;
namespace RegionalArt {
 FString MaterialFor(int Region,const FString& Name,const FString& Original,float Height=0);
 FString LeverFor(int Region);
 FString KeyFor(const FString& Name);
 FString FurnishingFor(int Region,int Variant);
 void Decorate(ADungeonController* Host,AActor* Root);
}
