#include "DungeonSewerArt.h"
#include "DungeonGame.h"
#include "DungeonRenderUtils.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"

FString SewerArt::MaterialFor(const FString& Name,const FString& Original){
 FString Role;
 if(Original=="I_Ember"||Original=="I_Soot")return Original;
 if(Original=="I_Water")Role="MurkyWater";
 else if(Original=="I_Rust"||Name=="Door"||Name=="Grate")Role="CorrodedIron";
 else if(Name=="Floor")Role="MaintenanceFlags";
 else if(Name=="Vault")Role="VaultBrick";
 else if(Name=="Arch"||Name=="Trim"||Name=="Rubble")Role="LimestoneCoping";
 else Role="ReservoirAshlar";
 return "/Game/OldCitySewers/Materials/MI_Game_"+Role+".MI_Game_"+Role;
}
// Presentation only: do not mutate dungeon records or add collision.
void SewerArt::Decorate(ADungeonController* Host,AActor* Root){
 auto M=Host->Model.Get();auto S=M->State;auto& D=M->Floors[S->Floor];if(D.RegionIndex!=1)return;
 auto& R=S->Floors[S->Floor];int W=D.Rows[0].Len(),Wall=0,PropCount=0,DecalCount=0;
 TMap<FString,UInstancedStaticMeshComponent*> Batches;
 auto Prop=[&](FString Name,FVector P,FRotator Rot=FRotator::ZeroRotator,FVector Scale=FVector::OneVector){
  auto C=Batches.FindRef(Name);
  if(!C){
   FString AssetPath=Name=="BrickSkirt"?FString("/Engine/BasicShapes/Cube.Cube"):"/Game/OldCitySewers/Meshes/SM_OCS_"+Name+".SM_OCS_"+Name;
   auto Asset=LoadObject<UStaticMesh>(nullptr,*AssetPath);
   if(!Asset){UE_LOG(LogTemp,Error,TEXT("OCS missing prop %s"),*Name);return;}
   C=DungeonBatch(Root,*FString((Name=="BrickSkirt"?"Sewer_":"OCS_")+Name));C->SetStaticMesh(Asset);C->SetupAttachment(Root->GetRootComponent());
   C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCastShadow(Name!="BrickSkirt");
   if(Name=="BrickSkirt")C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/OldCitySewers/Materials/MI_Game_ConduitBrick.MI_Game_ConduitBrick")));
   Batches.Add(Name,C);
  }
  C->AddInstance(FTransform(Rot,P,Scale));++PropCount;
 };
 auto Decal=[&](FString Name,FVector P,FVector Inward,FVector Size){
  auto Mat=LoadObject<UMaterialInterface>(nullptr,*("/Game/OldCitySewers/Materials/MI_"+Name+".MI_"+Name));
  if(!Mat){UE_LOG(LogTemp,Error,TEXT("OCS missing decal %s"),*Name);return;}
  auto C=NewObject<UDecalComponent>(Root,*FString::Printf(TEXT("OCS_%s_%d"),*Name,DecalCount));C->SetupAttachment(Root->GetRootComponent());
  C->SetDecalMaterial(Mat);C->DecalSize=Size;C->SetFadeScreenSize(.006f);C->RegisterComponent();C->SetWorldLocationAndRotation(P,Inward.Rotation());++DecalCount;
 };
 for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<W;++X){
  if(D.Rows[Y][X]=='#'||D.Rows[Y][X]=='~')continue;
  if((X+Y)%3==0)Prop("DrainGrate",FVector(X*400-160,Y*400,4),FRotator::ZeroRotator,FVector(.55,1,1));
 }
 for(const auto& E:R.Boundaries){
  if(E.B<0||E.Kind!="Wall")continue;
  FVector A(E.A%W*400,E.A/W*400,0),B(E.B%W*400,E.B/W*400,0),P=(A+B)*.5f,Inward=(A-P).GetSafeNormal();++Wall;
  Prop("BrickSkirt",P+Inward*31+FVector(0,0,35),FRotator(0,Inward.Rotation().Yaw-90,0),FVector(4,.02,.7));
  if(Wall%7==0)Prop("PipeOutlet",P+Inward*45+FVector(0,0,150),FRotator(0,Inward.Rotation().Yaw-90,0));
  if(Wall%5==0)Prop("HangingMoss",P+Inward*29+FVector(0,0,310),FRotator(0,Inward.Rotation().Yaw-90,0));
  if(Wall%3==0){
   static const TCHAR* Names[]={TEXT("RisingDamp"),TEXT("MineralBloom"),TEXT("PipeJointLeak"),TEXT("MortarSpall")};
   int Variant=(Wall/3)%4;float Z=Variant==0?48:Variant==1?72:Variant==2?95:170;
   Decal(Names[Variant],P+Inward*26+FVector(0,0,Z),Inward,FVector(12,Variant==0?100:55,Variant==3?25:50));
  }
 }
 FinishDungeonBatches(Batches);
 UE_LOG(LogTemp,Display,TEXT("OCS_ART_ACTIVE floor=%d prop_instances=%d decal_components=%d collision=unchanged"),S->Floor,PropCount,DecalCount);
}
