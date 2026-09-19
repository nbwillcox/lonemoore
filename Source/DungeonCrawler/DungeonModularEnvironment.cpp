#include "DungeonGame.h"
#include "DungeonPresentation.h"
#include "DungeonRoomKit.h"
#include "DungeonRenderUtils.h"
#include "DungeonRegionalArt.h"
#include "DungeonRoomDressing.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"

namespace {
const TCHAR* RegionCodes[]={TEXT("Cathedral"),TEXT("Sewer"),TEXT("Catacombs"),TEXT("Warrens"),TEXT("Crypts"),TEXT("Fortress"),TEXT("Deep"),TEXT("Infernal"),TEXT("Hell")};
const TCHAR* SurfaceRoles[]={TEXT("Wall"),TEXT("Floor"),TEXT("Vault"),TEXT("Trim"),TEXT("Iron")};
FString RoomMaterial(int Region,int SurfaceIndex){
 const FString Name=FString("MI_RK_")+RegionCodes[FMath::Clamp(Region,0,8)]+"_"+SurfaceRoles[SurfaceIndex];
 return "/Game/RoomKit/Materials/"+Name+"."+Name;
}
}

void ADungeonController::BuildModularArchitecture(){
 auto S=Model->State;const auto& D=Model->Floors[S->Floor];const auto& R=S->Floors[S->Floor];const int W=D.Rows[0].Len(),Region=D.RegionIndex;
 DoorVisuals.Empty();Flames.Empty();Dust=nullptr;DustOrigins.Empty();PendingDressingRoot.Reset();
 auto Root=GetWorld()->SpawnActor<AActor>();Root->Tags.Add("RoomKitArchitecture");Scenery.Add(Root);
 auto RC=NewObject<USceneComponent>(Root);Root->SetRootComponent(RC);RC->SetMobility(EComponentMobility::Static);RC->RegisterComponent();
 TMap<FString,UInstancedStaticMeshComponent*> Batches;
 UMaterialInterface* Materials[5];for(int I=0;I<5;++I)Materials[I]=LoadObject<UMaterialInterface>(nullptr,*RoomMaterial(Region,I));
 auto Instance=[&](const FString& Path,FVector P,FRotator Rot=FRotator::ZeroRotator,FVector Scale=FVector::OneVector,int SurfaceIndex=-1,bool RoomSlots=false){
  const FString Key=Path+FString::Printf(TEXT("_%d_%d"),SurfaceIndex,RoomSlots);auto C=Batches.FindRef(Key);
  if(!C){auto SM=LoadObject<UStaticMesh>(nullptr,*Path);if(!SM){UE_LOG(LogTemp,Error,TEXT("ROOM_KIT_MISSING %s"),*Path);return;}
   C=DungeonBatch(Root,*FString::Printf(TEXT("RoomKitBatch_%d"),Batches.Num()));C->SetStaticMesh(SM);C->SetupAttachment(RC);
   C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCastShadow(true);
   // Full room shells remain available to occlude corridors; never distance-cull a wall.
   if(RoomSlots){for(int I=0;I<C->GetNumMaterials()&&I<5;++I)C->SetMaterial(I,Materials[I]);}
   else if(SurfaceIndex==-2)C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Game/Materials/M_I_Ember.M_I_Ember")));
   else if(SurfaceIndex<=-3){const TCHAR* PitMaterial=SurfaceIndex==-3?TEXT("/Game/CampaignExpansion/Materials/M_FirePit.M_FirePit"):SurfaceIndex==-4?TEXT("/Game/OldCitySewers/Materials/MI_Game_MurkyWater.MI_Game_MurkyWater"):TEXT("/Game/CampaignExpansion/Materials/M_Chasm.M_Chasm");C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,PitMaterial));C->SetCastShadow(false);}
   else if(SurfaceIndex>=0)for(int I=0;I<C->GetNumMaterials();++I)C->SetMaterial(I,Materials[SurfaceIndex]);
   Batches.Add(Key,C);
  }
  C->AddInstance(FTransform(Rot,P,Scale));
 };
 auto Collision=[&](FVector P,FVector Extent,FRotator Rot=FRotator::ZeroRotator){
  auto C=NewObject<UBoxComponent>(Root);C->SetupAttachment(RC);C->SetMobility(EComponentMobility::Static);C->SetBoxExtent(Extent);
  C->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);C->SetCollisionResponseToAllChannels(ECR_Block);C->SetWorldLocationAndRotation(P,Rot);C->RegisterComponent();
 };
 auto Light=[&](FVector P,float Intensity,float Radius,FLinearColor Color){
  auto A=GetWorld()->SpawnActor<APointLight>(P,FRotator::ZeroRotator);A->SetMobility(EComponentMobility::Movable);auto L=A->PointLightComponent;
  L->SetIntensity(Intensity);L->SetAttenuationRadius(Radius);L->SetLightColor(Color);L->SetCastShadows(false);ConfigureDungeonLight(L);Scenery.Add(A);
 };
 const FVector Direction[]={FVector(0,-1,0),FVector(1,0,0),FVector(0,1,0),FVector(-1,0,0)};
 TSet<FString> Caps;int Lights=0,Props=0;
 for(const auto& Placement:R.RoomPlacements){
  const auto* Room=DungeonRoomKit::Find(Placement.Id);if(!Room)continue;
  const FVector Origin(Placement.X*400,Placement.Y*400,0);const FRotator Rotation(0,Placement.Rotation*90,0);
  // No non-uniform room scaling: geometry, metric UVs and navigation share one contract.
  Instance(Room->Mesh,Origin,Rotation,FVector::OneVector,-1,true);
  const int PortMask=DungeonRoomKit::RotateMask(Room->SocketMask,Placement.Rotation);
  for(int Dir=0;Dir<4;++Dir)if((PortMask&(1<<Dir))&&!(Placement.ActiveSockets&(1<<Dir))){
   FVector P=Origin+Direction[Dir]*1400;FString Key=FString::Printf(TEXT("%d_%d"),FMath::RoundToInt(P.X),FMath::RoundToInt(P.Y));
   if(Caps.Contains(Key))continue;Caps.Add(Key);
   Instance("/Game/RoomKit/Meshes/SM_RK_portal_cap.SM_RK_portal_cap",P,FRotator(0,Dir%2?90:0,0),FVector(1,1,640.f/600.f),-1,true);
  }
  for(const auto& Marker:Room->Lights){
   const FVector P=Origin+Rotation.RotateVector(Marker.Position);const FRotator Facing(0,Rotation.Yaw+Marker.Yaw,0);
   // The fixture's authored base is at the floor; the marker records its flame centre.
   Instance("/Game/Game/Environment/SM_I_Brazier.SM_I_Brazier",P-FVector(0,0,155),Facing,FVector::OneVector,4);
   Instance("/Game/Game/Environment/SM_I_Brazier.SM_I_Brazier",P-FVector(0,0,30),Facing,FVector(.22,.22,.15),-2);
   Light(P,9000,1050,FLinearColor(1,.49,.17));++Lights;
  }
  // Restrained indirect fill keeps materials legible without flattening the torch pools.
  Light(Origin+FVector(0,0,360),Room->Id=="boss_approach"?DungeonPresentation::BossApproachFillIntensity:DungeonPresentation::RoomFillIntensity,2250,FLinearColor(.56,.59,.62));++Lights;
  for(const auto& Marker:Room->Props){
   FString Path;FVector Local=Marker.Position;
   if(Region==1){const TCHAR* Ids[]={TEXT("PipeOutlet"),TEXT("HangingMoss"),TEXT("DrainGrate")};const int Variant=FMath::Abs(Marker.Variant)%3;FString Id=FString("SM_OCS_")+Ids[Variant];Path="/Game/OldCitySewers/Meshes/"+Id+"."+Id;Local.Z+=Variant==0?150.f:Variant==1?310.f:3.f;}
   else{FString Id="SM_"+RegionalArt::FurnishingFor(Region,Marker.Variant);Path="/Game/RegionalArt/Meshes/"+Id+"."+Id;}
   Instance(Path,Origin+Rotation.RotateVector(Local),FRotator(0,Rotation.Yaw+Marker.Yaw,0));++Props;
  }
 }
 // Collision is generated from the same socket graph, not from Nanite fallback hulls.
 // Merge coplanar logical walls into runs; these invisible boxes cannot cause z-fighting.
 struct FRun{bool East=false;int Plane=0;TArray<int> Cells;};TMap<FString,FRun> Runs;
 for(const auto& E:R.Boundaries){
  const int AX=E.A%W,AY=E.A/W,BX=E.B%W,BY=E.B/W;const bool East=AX!=BX;
  const FVector P((AX+BX)*200,(AY+BY)*200,0);const FRotator Rot(0,East?90:0,0);
  if(E.Kind=="Wall"||E.Kind=="Pit"){
   if(E.Kind=="Pit"&&D.Rows[AY][AX]=='~'&&D.Rows[BY][BX]=='~')continue;
   const int Plane=East?AX+BX:AY+BY;auto& Run=Runs.FindOrAdd(FString::Printf(TEXT("%d_%d"),East,Plane));Run.East=East;Run.Plane=Plane;Run.Cells.AddUnique(East?AY:AX);
  }else if(E.Kind!="Open"){
   Instance("/Game/Game/Environment/SM_I_Arch.SM_I_Arch",P,Rot,FVector::OneVector,3);
   Instance("/Engine/BasicShapes/Cube.Cube",P+FVector(0,0,600),Rot,FVector(4,.36,1.6),0);
   Collision(P+Rot.RotateVector(FVector(-180,0,400)),FVector(20,25,400),Rot);
   Collision(P+Rot.RotateVector(FVector(180,0,400)),FVector(20,25,400),Rot);
   Collision(P+FVector(0,0,750),FVector(200,25,250),Rot);
   auto A=GetWorld()->SpawnActor<AStaticMeshActor>();A->SetMobility(EComponentMobility::Movable);auto C=A->GetStaticMeshComponent();
   const FString Name=E.Kind=="Secret"?"Wall":E.Kind=="Bars"?"Grate":"Door";
   C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*("/Game/Game/Environment/SM_I_"+Name+".SM_I_"+Name)));C->SetMaterial(0,Materials[E.Kind=="Secret"?0:4]);
   C->SetCollisionEnabled(R.OpenDoors.Contains(E.DoorId)?ECollisionEnabled::NoCollision:ECollisionEnabled::QueryAndPhysics);C->SetCollisionResponseToAllChannels(ECR_Block);
   A->SetActorLocation(P+FVector(0,0,R.OpenDoors.Contains(E.DoorId)?520:0));A->SetActorHiddenInGame(R.OpenDoors.Contains(E.DoorId));A->SetActorRotation(Rot);Scenery.Add(A);DoorVisuals.Add({A,E.DoorId,P});
  }
 }
 for(auto& Pair:Runs){auto& Run=Pair.Value;Run.Cells.Sort();for(int I=0;I<Run.Cells.Num();){int Start=Run.Cells[I],End=Start;while(++I<Run.Cells.Num()&&Run.Cells[I]==End+1)End=Run.Cells[I];
  FVector P=Run.East?FVector(Run.Plane*200,(Start+End)*200,400):FVector((Start+End)*200,Run.Plane*200,400);
  Collision(P,FVector((End-Start+1)*200,18,400),FRotator(0,Run.East?90:0,0));
 }}
 for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<W;){
  if(D.Rows[Y][X]=='#'||D.Rows[Y][X]=='~'){++X;continue;}const int Start=X;while(X+1<W&&D.Rows[Y][X+1]!='#'&&D.Rows[Y][X+1]!='~')++X;
  Collision(FVector((Start+X)*200,Y*400,-20),FVector((X-Start+1)*200,200,20));++X;
 }
 // Each authored bridge owns its basin type; the deck and rail own z=0.
 for(const auto& Placement:R.RoomPlacements){
  const auto* Room=DungeonRoomKit::Find(Placement.Id);if(!Room)continue;
  const FRotator Rotation(0,Placement.Rotation*90,0);const FVector Origin(Placement.X*400,Placement.Y*400,0);
  const int SurfaceIndex=Placement.Id=="fire_bridge"||(Placement.Id=="cistern_crossing"&&Region>=7)?-3:Placement.Id=="cistern_crossing"?-4:-5;
  bool HasPit=false;
  for(int Y=0;Y<7;++Y)for(int X=0;X<7;++X)if(Room->Rows[Y][X]=='~'){
   HasPit=true;Instance("/Engine/BasicShapes/Plane.Plane",Origin+Rotation.RotateVector(FVector((X-3)*400,(Y-3)*400,-850)),Rotation,FVector(4,4,1),SurfaceIndex);
  }
  if(HasPit&&SurfaceIndex==-3)Light(Origin+FVector(0,0,-420),50000,2000,FLinearColor(1,.12,.01));
 }
 FinishDungeonBatches(Batches);
 RoomDressing::Decorate(this,Root);
 UE_LOG(LogTemp,Display,TEXT("ROOM_KIT_ARCHITECTURE rooms=%d batches=%d sealed_ports=%d fixtures=%d lights=%d"),R.RoomPlacements.Num(),Batches.Num(),Caps.Num(),Props,Lights);
}
