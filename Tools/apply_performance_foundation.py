"""Apply the first performance refactor to the pre-pass source layout."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]/'Source/DungeonCrawler'
for name in ['DungeonExpansionEnvironment.cpp','DungeonEnvironment.cpp','DungeonSewerArt.cpp','DungeonRegionalArt.cpp']:
    p=r/name;s=p.read_text();s=s.replace('#include "DungeonGame.h"','#include "DungeonGame.h"\n#include "DungeonRenderUtils.h"')
    s=s.replace('Root->SetRootComponent(RC);RC->RegisterComponent();','Root->SetRootComponent(RC);RC->SetMobility(EComponentMobility::Static);RC->RegisterComponent();')
    s=s.replace('NewObject<UInstancedStaticMeshComponent>(Root,','DungeonBatch(Root,')
    s=s.replace('C=NewObject<UInstancedStaticMeshComponent>(Root);','C=DungeonBatch(Root);')
    # Delay registration until all instances/materials are populated.
    s=s.replace('C->SetCastShadow(true);C->SetCullDistances(0,18000);C->RegisterComponent();','C->SetCastShadow(true);C->SetCullDistances(0,22000);')
    s=s.replace('C->SetCastShadow(true);C->RegisterComponent();','C->SetCastShadow(true);')
    s=s.replace('V->SetCastShadow(true);V->RegisterComponent();','V->SetCastShadow(true);')
    s=s.replace('C->RegisterComponent();Batches.Add','Batches.Add')
    if name in ['DungeonExpansionEnvironment.cpp','DungeonEnvironment.cpp']:
        s=s.replace('SewerArt::Decorate(this,Root);RegionalArt::Decorate(this,Root);','FinishDungeonBatches(Batches);SewerArt::Decorate(this,Root);RegionalArt::Decorate(this,Root);')
    else:
        marker=' UE_LOG(LogTemp,Display,TEXT("'+('OCS_ART_ACTIVE' if name=='DungeonSewerArt.cpp' else 'REGIONAL_ART')
        assert marker in s;s=s.replace(marker,' FinishDungeonBatches(Batches);\n'+marker)
    if name=='DungeonExpansionEnvironment.cpp':
        s=s.replace('L->SetCastShadows(false);Scenery.Add(A);','L->SetCastShadows(false);ConfigureDungeonLight(L);Scenery.Add(A);')
    if name=='DungeonEnvironment.cpp':
        s=s.replace('Light->PointLightComponent->SetCastShadows(false);Scenery.Add(Light);','Light->PointLightComponent->SetCastShadows(false);ConfigureDungeonLight(Light->PointLightComponent);Scenery.Add(Light);')
        s=s.replace('Fill->PointLightComponent->SetCastShadows(false);Scenery.Add(Fill);','Fill->PointLightComponent->SetCastShadows(false);ConfigureDungeonLight(Fill->PointLightComponent);Scenery.Add(Fill);')
    p.write_text(s)

p=r/'DungeonGame.cpp';s=p.read_text();s=s.replace('#include "DungeonGame.h"','#include "DungeonGame.h"\n#include "DungeonRenderUtils.h"')
old=' for(auto A:Scenery)if(IsValid(A))A->Destroy();Scenery.Empty();Billboards.Empty();HoverKeys.Empty();DoorVisuals.Empty();Flames.Empty();WorldEnemyVisuals.Empty();PlayerTorch.Reset();PlayerTorchFill.Reset();LastX=-1;auto S=Model->State;if(S->Party.IsEmpty()||S->InTown)return;'
new=''' const bool KeepArchitecture=ReuseArchitecture();const double Started=FPlatformTime::Seconds();
 for(auto A:Scenery)if(IsValid(A)&&(!KeepArchitecture||!Architecture.Contains(A)))A->Destroy();
 if(!KeepArchitecture){Architecture.Empty();DoorVisuals.Empty();Flames.Empty();Dust=nullptr;DustOrigins.Empty();BuiltState.Reset();BuiltFloor=-1;}
 Scenery=Architecture;Billboards.Empty();HoverKeys.Empty();WorldEnemyVisuals.Empty();PlayerTorch.Reset();PlayerTorchFill.Reset();LastVisualCamera=FVector(1.e20);auto S=Model->State;if(S->Party.IsEmpty()||S->InTown){LastX=-1;return;}
 if(!KeepArchitecture)LastX=-1;'''
assert old in s;s=s.replace(old,new)
s=s.replace(' BuildArchitecture();',''' if(!KeepArchitecture){int First=Scenery.Num();BuildArchitecture();for(int I=First;I<Scenery.Num();++I)Architecture.Add(Scenery[I]);BuiltState=S;BuiltFloor=S->Floor;BuiltSeed=R.LayoutSeed;BuiltVersion=R.ContentVersion;++ArchitectureBuilds;}
 UE_LOG(LogTemp,Display,TEXT("DUNGEON_WORLD_REFRESH architecture=%s build_count=%d cpu_ms=%.3f"),KeepArchitecture?TEXT("reused"):TEXT("built"),ArchitectureBuilds,(FPlatformTime::Seconds()-Started)*1000);''')
marker='void ADungeonController::RebuildWorld(){'
s=s.replace(marker,'''bool ADungeonController::ReuseArchitecture() const{
 auto S=Model->State;if(!S||S->InTown||S->Party.IsEmpty()||Architecture.IsEmpty()||BuiltState.Get()!=S||BuiltFloor!=S->Floor)return false;
 const auto& R=S->Floors[S->Floor];return BuiltSeed==R.LayoutSeed&&BuiltVersion==R.ContentVersion;
}
'''+marker)
s=s.replace('L->PointLightComponent->SetCastShadows(false);Scenery.Add(L);','L->PointLightComponent->SetCastShadows(false);ConfigureDungeonLight(L->PointLightComponent);Scenery.Add(L);')
s=s.replace('A->GetStaticMeshComponent()->SetCastShadow(false);float Height','A->GetStaticMeshComponent()->SetCastShadow(false);A->GetStaticMeshComponent()->SetCullDistance(16000);float Height')
s=s.replace(' for(auto& K:HoverKeys)if(K.Actor.IsValid()){K.Actor->SetActorHiddenInGame(Model->Combat);',''' const bool CameraChanged=!LastVisualCamera.Equals(DungeonCamera->GetActorLocation(),.05f)||LastVisualCombat!=Model->Combat;
 if(!CameraChanged)return;LastVisualCamera=DungeonCamera->GetActorLocation();LastVisualCombat=Model->Combat;
 for(auto& K:HoverKeys)if(K.Actor.IsValid()){K.Actor->SetActorHiddenInGame(Model->Combat);''')
s=s.replace('  Enemy.Material->SetScalarParameterValue(TEXT("WorldLight"),Light);','  Enemy.Material->SetScalarParameterValue(TEXT("WorldLight"),Light);')
p.write_text(s)

p=r/'DungeonEnvironment.cpp';s=p.read_text()
s=s.replace('if(K.Actor.IsValid())K.Actor->SetActorLocation','if(K.Actor.IsValid()&&FVector::DistSquared(K.Origin,DungeonCamera->GetActorLocation())<FMath::Square(8000.f))K.Actor->SetActorLocation')
# Do not upload an unchanged door transform/collision state every frame.
s=s.replace('V.Actor->SetActorLocation(V.Closed+FVector(0,0,520*Alpha));','if(FMath::IsNearlyEqual(Alpha,V.LastAlpha))continue;V.LastAlpha=Alpha;V.Actor->SetActorLocation(V.Closed+FVector(0,0,520*Alpha));')
s=s.replace('#include "Engine/StaticMesh.h"','#include "Engine/StaticMesh.h"\n#include "Camera/CameraActor.h"')
p.write_text(s)
print('PERFORMANCE_FOUNDATION_APPLIED')
