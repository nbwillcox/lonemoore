#pragma once
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"

// Build immutable dungeon batches once. HISM supplies a cluster tree for ordinary
// meshes; Nanite meshes retain their GPU cluster culling. Never rebuild per instance.
inline UInstancedStaticMeshComponent* DungeonBatch(AActor* Owner, FName Name=NAME_None){
 auto C=NewObject<UHierarchicalInstancedStaticMeshComponent>(Owner,Name);
 C->SetMobility(EComponentMobility::Static);
 C->bAutoRebuildTreeOnInstanceChanges=false;
 return C;
}
inline void FinishDungeonBatches(TMap<FString,UInstancedStaticMeshComponent*>& Batches){
 for(auto& Pair:Batches){auto C=CastChecked<UHierarchicalInstancedStaticMeshComponent>(Pair.Value);
  C->RegisterComponent();C->BuildTreeIfOutdated(false,true);
 }
}
inline void ConfigureDungeonLight(UPointLightComponent* Light){
 // Preserve all nearby illumination and fade distant local lights smoothly.
 Light->SetMaxDrawDistance(Light->AttenuationRadius+8000.f);
 Light->SetMaxDistanceFadeRange(1600.f);
 Light->SetAffectTranslucentLighting(false);
}
