#include "DungeonGame.h"
#include "DungeonPresentation.h"
#include "DungeonRenderUtils.h"
#include "DungeonSewerArt.h"
#include "DungeonRegionalArt.h"
#include "DungeonIdentityArt.h"
#include "Engine/StaticMesh.h"
#include "Camera/CameraActor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "Materials/MaterialInstanceDynamic.h"

void ADungeonController::BuildArchitecture(){
 if(Model->State->Floors[Model->State->Floor].ContentVersion==4){BuildModularArchitecture();return;}
 if(Model->State->Floors[Model->State->Floor].ContentVersion==3){BuildExpandedArchitecture();return;}
 auto S=Model->State;auto& D=Model->Floors[S->Floor];auto& R=S->Floors[S->Floor];int W=D.Rows[0].Len();DoorVisuals.Empty();Flames.Empty();Dust=nullptr;DustOrigins.Empty();
 FString Stone=D.RegionIndex==1?"I_Sewer":D.RegionIndex==2||D.RegionIndex==4?"I_Crypt":D.RegionIndex==5||D.RegionIndex==6?"I_Fortress":D.RegionIndex>=7?"I_Infernal":"I_Limestone";
 auto Root=GetWorld()->SpawnActor<AActor>();Scenery.Add(Root);auto RC=NewObject<USceneComponent>(Root);Root->SetRootComponent(RC);RC->SetMobility(EComponentMobility::Static);RC->RegisterComponent();
 TMap<FString,UInstancedStaticMeshComponent*> Batches;
 auto Mesh=[&](FString Name,FVector P,FRotator Rot=FRotator::ZeroRotator,FVector Scale=FVector::OneVector,FString Mat="",bool Collide=true){
  if(Mat.IsEmpty())Mat=Stone;if(D.RegionIndex==1)Mat=SewerArt::MaterialFor(Name,Mat);else Mat=RegionalArt::MaterialFor(D.RegionIndex,Name,Mat,P.Z);FString Key=Name+Mat+(Collide?"solid":"scenic");auto C=Batches.FindRef(Key);
  if(!C){auto SM=LoadObject<UStaticMesh>(nullptr,*("/Game/Game/Environment/SM_I_"+Name+".SM_I_"+Name));if(!SM){UE_LOG(LogTemp,Error,TEXT("Missing integrity module %s"),*Name);return;}
   C=DungeonBatch(Root);C->SetStaticMesh(SM);C->SetupAttachment(RC);C->SetCollisionEnabled(Collide?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);C->SetCollisionResponseToAllChannels(ECR_Block);C->SetCastShadow(true);
   FString MaterialPath=Mat.StartsWith("/Game/")?Mat:"/Game/Game/Materials/M_"+Mat+".M_"+Mat;
   if(auto Material=LoadObject<UMaterialInterface>(nullptr,*MaterialPath))C->SetMaterial(0,Material);Batches.Add(Key,C);
  }C->AddInstance(FTransform(Rot,P,Scale));
  const FString VisualPath=IdentityArt::MeshFor(D.RegionIndex,S->Floor,Name,P.Z);
  if(!VisualPath.IsEmpty()){
   const FString VisualKey="ID_"+VisualPath;auto V=Batches.FindRef(VisualKey);
   if(!V){
    if(auto VisualMesh=LoadObject<UStaticMesh>(nullptr,*VisualPath)){
     V=DungeonBatch(Root,*FString("ID_"+VisualMesh->GetName()));V->SetStaticMesh(VisualMesh);V->SetupAttachment(RC);
     if(Name=="Wall"||Name=="Niche")for(int I=0;I<V->GetNumMaterials();++I)if(auto Authored=V->GetMaterial(I)){
      const FString WorldPath=Authored->GetPathName().Replace(TEXT("MI_UV_"),TEXT("MI_"));
      if(auto WorldMaterial=LoadObject<UMaterialInterface>(nullptr,*WorldPath))V->SetMaterial(I,WorldMaterial);
     }
     V->SetCollisionEnabled(ECollisionEnabled::NoCollision);V->SetCastShadow(true);Batches.Add(VisualKey,V);
    }else UE_LOG(LogTemp,Error,TEXT("Missing regional identity mesh %s"),*VisualPath);
   }
   if(V){C->SetVisibility(false);C->SetCastShadow(false);V->AddInstance(FTransform(Rot,P,Scale));}
  }

 };
 auto Seal=[&](FVector Pos,FVector Extent,FRotator Rot){auto Box=NewObject<UBoxComponent>(Root);Box->SetupAttachment(RC);Box->SetBoxExtent(Extent);Box->SetWorldLocationAndRotation(Pos,Rot);Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Box->SetCollisionResponseToAllChannels(ECR_Block);Box->SetHiddenInGame(true);Box->RegisterComponent();};
 for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<W;++X){if(D.Rows[Y][X]=='#')continue;int C=Y*W+X;FVector P(X*400,Y*400,0);FString Purpose=R.Rooms.FindRef(C);bool Hall=Purpose=="Ossuary"||Purpose=="Cistern"||Purpose=="Lower sanctuary"||Purpose=="Ritual sanctuary";
  Mesh("Floor",P);Seal(P-FVector(0,0,10),FVector(200,200,10),FRotator::ZeroRotator);Mesh("Vault",P+FVector(0,0,Hall?170:0));Mesh("Floor",P+FVector(0,0,Hall?760:590));Seal(P+FVector(0,0,Hall?760:590),FVector(200,200,12),FRotator::ZeroRotator);
  if(D.RegionIndex==1){Mesh("Floor",P+FVector(-160,0,1),FRotator::ZeroRotator,FVector(.13,1,.2),"I_Water",false);}
  // Purposeful landmarks sit beside, never across, the central walking footprint.
  if(Purpose=="Ossuary"&&X%3==0&&Y%3==0)Mesh("Tomb",P+FVector(125,90,0),FRotator(0,90,0),FVector(.65,.65,.65),"I_Bone");
  if(Purpose=="Abandoned stores"&&X%3==1&&Y%3==1)Mesh("Camp",P+FVector(120,105,0),FRotator::ZeroRotator,FVector::OneVector,"Wood",false);
  if(D.RegionIndex==3&&(X+Y)%7==0)Mesh("Mushrooms",P+FVector(160,120,0),FRotator::ZeroRotator,FVector::OneVector,"I_Fungus",false);
  if((Purpose=="Lower sanctuary"||Purpose=="Ritual sanctuary")&&X%4==0&&Y%4==0)Mesh(D.RegionIndex>=7?"Altar":"Statue",P+FVector(135,125,0),FRotator::ZeroRotator,FVector(.75,.75,1.3));
 }
 for(const auto& E:R.Boundaries){int AX=E.A%W,AY=E.A/W;int BX=E.B>=0?E.B%W:AX,BY=E.B>=0?E.B/W:AY;FVector P((AX+BX)*200,(AY+BY)*200,0);bool East=AX!=BX;FRotator Rot(0,East?90:0,0);
  if(E.B<0)continue;bool Tall=R.Rooms.FindRef(E.A)=="Ossuary"||R.Rooms.FindRef(E.A)=="Cistern"||R.ProtectedCells.Contains(E.A);FVector Scale(1,1,Tall?1.33:1);
  if(E.Kind=="Wall"){
   bool Niche=(D.RegionIndex==2||D.RegionIndex==4||D.RegionIndex==0)&&E.A%3==0;
   // Face recesses toward the cell so the playable camera sees actual depth.
   if((East&&BX>AX)||(!East&&BY<AY))Rot.Yaw+=180;
   Mesh(Niche?"Niche":"Wall",P,Rot,Scale);Seal(P+FVector(0,0,345),FVector(200,23,345),Rot);
   if(E.A%5==0){FVector Inward=(FVector(AX*400,AY*400,0)-P).GetSafeNormal();Mesh("Brazier",P+Inward*70,FRotator::ZeroRotator,FVector::OneVector,"I_Rust",false);Mesh("Rubble",P+Inward*45+FVector(70,0,0),FRotator::ZeroRotator,FVector(.65,.65,.6),Stone,false);
    auto Light=GetWorld()->SpawnActor<APointLight>(P+Inward*100+FVector(0,0,155),FRotator::ZeroRotator);Light->SetMobility(EComponentMobility::Movable);Light->PointLightComponent->SetIntensity(7000);Light->PointLightComponent->SetAttenuationRadius(720);Light->PointLightComponent->SetLightColor(FLinearColor(1,.51,.18));Light->PointLightComponent->SetCastShadows(false);ConfigureDungeonLight(Light->PointLightComponent);Scenery.Add(Light);Flames.Add(Light);
    Mesh("Brazier",P+Inward*70+FVector(0,0,125),FRotator::ZeroRotator,FVector(.22,.22,.15),"I_Ember",false);
    Mesh("Trim",P+FVector(0,0,-80),Rot,FVector(.30,.25,1),"I_Soot",false);
   }
  }else if(E.Kind!="Open"){
   Mesh("Arch",P,Rot);Mesh("Wall",P+FVector(0,0,520),Rot,FVector(1,1,.35));
   auto A=GetWorld()->SpawnActor<AStaticMeshActor>();A->SetMobility(EComponentMobility::Movable);auto C=A->GetStaticMeshComponent();FString Name=E.Kind=="Secret"?"Wall":E.Kind=="Bars"?"Grate":"Door";
   C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*("/Game/Game/Environment/SM_I_"+Name+".SM_I_"+Name)));FString DoorMat=E.Kind=="Secret"?Stone:FString("I_Rust");FString DoorPath=D.RegionIndex==1?SewerArt::MaterialFor(Name,DoorMat):RegionalArt::MaterialFor(D.RegionIndex,Name,DoorMat);C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,*DoorPath));C->SetCollisionEnabled(R.OpenDoors.Contains(E.DoorId)?ECollisionEnabled::NoCollision:ECollisionEnabled::QueryAndPhysics);C->SetCollisionResponseToAllChannels(ECR_Block);A->SetActorLocation(P+FVector(0,0,R.OpenDoors.Contains(E.DoorId)?520:0));A->SetActorRotation(Rot);Scenery.Add(A);DoorVisuals.Add({A,E.DoorId,P});
   if(E.Kind=="Secret"){
    const FString VisualPath=IdentityArt::MeshFor(D.RegionIndex,S->Floor,"Wall");
    if(!VisualPath.IsEmpty())if(auto VisualMesh=LoadObject<UStaticMesh>(nullptr,*VisualPath)){
     auto V=NewObject<UStaticMeshComponent>(A,TEXT("ID_SecretVisual"));V->SetStaticMesh(VisualMesh);V->SetupAttachment(C);V->SetCollisionEnabled(ECollisionEnabled::NoCollision);V->RegisterComponent();
     C->SetVisibility(false,false);C->SetCastShadow(false);
    }
   }

  }
 }
 FinishDungeonBatches(Batches);if(WorldLoading)PendingDressingRoot=Root;else{SewerArt::Decorate(this,Root);RegionalArt::Decorate(this,Root);}
 TMap<FString,FVector> Centers;TMap<FString,int> Counts;for(const auto& Pair:R.Rooms){if(Pair.Value=="Passage"||Pair.Value=="Hidden burial")continue;Centers.FindOrAdd(Pair.Value)+=FVector(Pair.Key%W*400,Pair.Key/W*400,320);Counts.FindOrAdd(Pair.Value)++;}
 for(auto& Pair:Centers){FVector Center=Pair.Value/Counts[Pair.Key];auto Fill=GetWorld()->SpawnActor<APointLight>(Center,FRotator::ZeroRotator);Fill->SetMobility(EComponentMobility::Movable);Fill->PointLightComponent->SetIntensity(38000);Fill->PointLightComponent->SetAttenuationRadius(2300);Fill->PointLightComponent->SetLightColor(D.RegionIndex==1?FLinearColor(.40,.56,.52):FLinearColor(.64,.58,.45));Fill->PointLightComponent->SetCastShadows(false);ConfigureDungeonLight(Fill->PointLightComponent);Scenery.Add(Fill);
  FString SoundName=D.RegionIndex==1?"ambient_drips":D.RegionIndex>=7?"ambient_infernal":"ambient_stone";auto Sound=LoadObject<USoundBase>(nullptr,*("/Game/Game/Audio/A_"+SoundName+".A_"+SoundName));if(Sound){auto Audio=NewObject<UAudioComponent>(Root);Audio->SetupAttachment(RC);Audio->SetWorldLocation(Center);Audio->SetSound(Sound);Audio->bOverrideAttenuation=true;Audio->AttenuationOverrides.bAttenuate=true;Audio->AttenuationOverrides.FalloffDistance=1800;Audio->AttenuationOverrides.AttenuationShapeExtents=FVector(350);Audio->SetVolumeMultiplier(MasterVolume*EffectsVolume*.35f);Audio->RegisterComponent();Audio->Play();}
  for(int I=0;I<6;++I)DustOrigins.Add(Center+FVector(FMath::Sin(I*2.1)*420,FMath::Cos(I*3.2)*420,-90+I*28));
 }
 Dust=NewObject<UInstancedStaticMeshComponent>(Root);Dust->SetupAttachment(RC);Dust->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere")));Dust->SetCollisionEnabled(ECollisionEnabled::NoCollision);Dust->SetCastShadow(false);Dust->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Game/Materials/M_I_Bone.M_I_Bone")));Dust->RegisterComponent();for(auto P:DustOrigins)Dust->AddInstance(FTransform(FRotator::ZeroRotator,P,FVector(.012)));
}
void ADungeonController::TickAtmosphere(float Delta){
 if(Model->Screen=="Dungeon"){KeyHoverClock=FMath::Fmod(KeyHoverClock+Delta,600.f);for(auto& K:HoverKeys)if(K.Actor.IsValid()&&FVector::DistSquared(K.Origin,DungeonCamera->GetActorLocation())<FMath::Square(8000.f))K.Actor->SetActorLocation(K.Origin+FVector(0,0,10.f*FMath::Sin(KeyHoverClock*(2.f*PI/2.4f)+K.Phase)));}
 AtmosphereClock+=Delta;
 if(PlayerTorch.IsValid())PlayerTorch->PointLightComponent->SetIntensity(Brightness*(DungeonPresentation::PlayerTorchIntensity+850.f*FMath::Sin(AtmosphereClock*4.7f)+450.f*FMath::Sin(AtmosphereClock*8.3f)));for(auto& V:DoorVisuals)if(V.Actor.IsValid()){const auto& Record=Model->State->Floors[Model->State->Floor];float Alpha=Record.OpenDoors.Contains(V.Id)?1:Model->DoorOpening.FindRef(V.Id);if(FMath::IsNearlyEqual(Alpha,V.LastAlpha))continue;V.LastAlpha=Alpha;V.Actor->SetActorLocation(V.Closed+FVector(0,0,520*Alpha));if(Record.ContentVersion==4)V.Actor->SetActorHiddenInGame(Alpha>=1);V.Actor->GetStaticMeshComponent()->SetCollisionEnabled(Alpha>=1?ECollisionEnabled::NoCollision:ECollisionEnabled::QueryAndPhysics);}
 for(int I=0;I<Flames.Num();++I)if(Flames[I].IsValid())Flames[I]->PointLightComponent->SetIntensity(6800+450*FMath::Sin(AtmosphereClock*5.1+I*1.7)+250*FMath::Sin(AtmosphereClock*8.7+I));
 if(IsValid(Dust))for(int I=0;I<DustOrigins.Num();++I)Dust->UpdateInstanceTransform(I,FTransform(FRotator::ZeroRotator,DustOrigins[I]+FVector(FMath::Sin(AtmosphereClock*.17+I)*50,FMath::Cos(AtmosphereClock*.13+I)*35,FMath::Sin(AtmosphereClock*.23+I)*25),FVector(.012)),false,I==DustOrigins.Num()-1);
}
