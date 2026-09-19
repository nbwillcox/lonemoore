#include "DungeonGame.h"
#include "DungeonRenderUtils.h"
#include "DungeonSewerArt.h"
#include "DungeonRegionalArt.h"
#include "DungeonIdentityArt.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"

void ADungeonController::BuildExpandedArchitecture(){
 auto S=Model->State;auto& D=Model->Floors[S->Floor];auto& R=S->Floors[S->Floor];const int W=D.Rows[0].Len();const int Region=D.RegionIndex;
 DoorVisuals.Empty();Flames.Empty();Dust=nullptr;DustOrigins.Empty();
 auto Root=GetWorld()->SpawnActor<AActor>();Scenery.Add(Root);auto RC=NewObject<USceneComponent>(Root);Root->SetRootComponent(RC);RC->SetMobility(EComponentMobility::Static);RC->RegisterComponent();
 TMap<FString,UInstancedStaticMeshComponent*> Batches;
 auto Mesh=[&](FString Name,FVector P,FVector Scale=FVector::OneVector,FRotator Rot=FRotator::ZeroRotator,FString SurfaceRole="ReservoirAshlar",bool Solid=false){
  FString Key=Name+SurfaceRole+(Solid?"solid":"scenic");auto C=Batches.FindRef(Key);
  if(!C){FString Path=Name=="Cube"?"/Engine/BasicShapes/Cube.Cube":Name.StartsWith("/Game/")?Name:Name.StartsWith("REG_")?"/Game/RegionalArt/Meshes/SM_"+Name.Mid(4)+".SM_"+Name.Mid(4):Name.StartsWith("EX_")?"/Game/ExpansionPrototype/Meshes/SM_"+Name.Mid(3)+".SM_"+Name.Mid(3):Name.StartsWith("OCS_")?"/Game/OldCitySewers/Meshes/SM_"+Name+".SM_"+Name:"/Game/Game/Environment/SM_I_"+Name+".SM_I_"+Name;
   auto SM=LoadObject<UStaticMesh>(nullptr,*Path);if(!SM){UE_LOG(LogTemp,Error,TEXT("Expansion mesh missing: %s"),*Path);return;}
   C=DungeonBatch(Root,*FString("EXP_"+Key.Replace(TEXT("/"),TEXT("_")).Replace(TEXT("."),TEXT("_"))));C->SetStaticMesh(SM);C->SetupAttachment(RC);C->SetCollisionEnabled(Solid?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);C->SetCollisionResponseToAllChannels(ECR_Block);C->SetCastShadow(true);C->SetCullDistances(0,22000);
   FString Mat="/Game/OldCitySewers/Materials/MI_Game_"+SurfaceRole+".MI_Game_"+SurfaceRole;
   if(Region!=1){FString Role=SurfaceRole=="MaintenanceFlags"?"Floor":SurfaceRole=="VaultBrick"?"Vault":SurfaceRole=="LimestoneCoping"?"Trim":SurfaceRole=="CorrodedIron"?"Brazier":"Wall";Mat=RegionalArt::MaterialFor(Region,Role,"I_Limestone");
    if(SurfaceRole=="MurkyWater")Mat=Region>=7?"/Game/CampaignExpansion/Materials/M_FirePit.M_FirePit":"/Game/CampaignExpansion/Materials/M_Chasm.M_Chasm";
   }
   if(SurfaceRole!="Authored")if(auto M=LoadObject<UMaterialInterface>(nullptr,*Mat))C->SetMaterial(0,M);Batches.Add(Key,C);
  }C->AddInstance(FTransform(Rot,P,Scale));
 };
 auto Box=[&](FVector P,FVector Extent,FRotator Rot=FRotator::ZeroRotator){auto B=NewObject<UBoxComponent>(Root);B->SetupAttachment(RC);B->SetBoxExtent(Extent);B->SetWorldLocationAndRotation(P,Rot);B->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);B->SetCollisionResponseToAllChannels(ECR_Block);B->RegisterComponent();};
 auto Height=[&](int C){FString Purpose=R.Rooms.FindRef(C);if(Purpose=="Guardian arena")return 850;if(Purpose=="Sealed descent")return 560;int Best=MAX_int32,Z=560;
  for(auto V:R.Volumes){if(V.Purpose!=Purpose)continue;int Dist=FMath::Square(C%W-V.X)+FMath::Square(C/W-V.Y);if(Dist<Best){Best=Dist;Z=V.Purpose=="Great reservoir"?1400:FMath::Clamp(FMath::RoundToInt(V.Ceiling*.62f),600,950);}}return Z;};
 auto Light=[&](FVector P,float Intensity,float Radius,FLinearColor Color){auto A=GetWorld()->SpawnActor<APointLight>(P,FRotator::ZeroRotator);A->SetMobility(EComponentMobility::Movable);auto L=A->PointLightComponent;L->SetIntensity(Intensity);L->SetAttenuationRadius(Radius);L->SetLightColor(Color);L->SetCastShadows(false);ConfigureDungeonLight(L);Scenery.Add(A);};
 // Floors and ceilings span variable lengths; large rooms have uninterrupted headroom.
 for(int Y=1;Y<D.Rows.Num()-1;++Y)for(int X=1;X<W-1;){
  TCHAR T=D.Rows[Y][X];if(T=='#'){++X;continue;}int Start=X,Z=Height(Y*W+X);bool Reservoir=R.Rooms.FindRef(Y*W+X)=="Great reservoir";
  while(X+1<W-1&&D.Rows[Y][X+1]!='#'&&(D.Rows[Y][X+1]=='~')==(T=='~')&&Height(Y*W+X+1)==Z&&(R.Rooms.FindRef(Y*W+X+1)=="Great reservoir")==Reservoir)++X;
  int Count=X-Start+1;FVector P((Start+X)*200,Y*400,0);
  if(T=='~')Mesh("Cube",P+FVector(0,0,-850),FVector(Count*4,4,.2),FRotator::ZeroRotator,"MurkyWater");
  else {if(Region!=1&&!Reservoir){for(int CellX=Start;CellX<=X;++CellX)Mesh(IdentityArt::MeshFor(Region,S->Floor,"Floor"),FVector(CellX*400,Y*400,0),FVector(400.f/400.2f,400.f/400.2f,1),FRotator::ZeroRotator,"Authored");}else Mesh(Reservoir?"EX_BridgeDeck":"Floor",P,FVector(Count,1,1),FRotator::ZeroRotator,"MaintenanceFlags");Box(P-FVector(0,0,22),FVector(Count*200,200,22));}
  Mesh("Cube",P+FVector(0,0,Z),FVector(Count*4,4,.3),FRotator::ZeroRotator,"VaultBrick");++X;
 }
 // Join contiguous wall edges into runs with their own length and height.
 struct FWallRun{bool East=false;int Plane=0,Height=650;TArray<int> Along;};TMap<FString,FWallRun> Runs;
 int Torch=0,Furniture=0,SupportCount=0;
 for(const auto& E:R.Boundaries){int AX=E.A%W,AY=E.A/W,BX=E.B%W,BY=E.B/W;bool East=AX!=BX;FVector P((AX+BX)*200,(AY+BY)*200,0);FRotator Rot(0,East?90:0,0);
  if(E.Kind=="Wall"){
   int Z=Height(E.A),Plane=East?AX+BX:AY+BY,Along=East?AY:AX;FString Key=FString::Printf(TEXT("%d_%d_%d"),East,Plane,Z);auto& Run=Runs.FindOrAdd(Key);Run.East=East;Run.Plane=Plane;Run.Height=Z;Run.Along.Add(Along);
   if(D.Rows[AY][AX]!='~'){
    FVector Inward=(FVector(AX*400,AY*400,0)-P).GetSafeNormal();
    if(Region!=1){FString Facade=IdentityArt::MeshFor(Region,S->Floor,(Region==0||Region==2||Region==4)&&E.A%3==0?"Niche":"Wall");if(!Facade.IsEmpty())Mesh(Facade,P+Inward*26,FVector(1,1,Z/520.f),FRotator(0,Inward.Rotation().Yaw-90,0),"Authored");}
    if(E.A%3==0){Mesh("Cube",P+Inward*48+FVector(0,0,Z*.5f),FVector(.7,.7,Z/100.f),FRotator::ZeroRotator,"LimestoneCoping");Mesh("Cube",P+Inward*48+FVector(0,0,28),FVector(.95,.95,.56),FRotator::ZeroRotator,"LimestoneCoping");}
    if(D.Rows[AY][AX]=='.'&&E.A%4==1){
     FString Purpose=R.Rooms.FindRef(E.A);FString Furnishing=Purpose=="Abandoned stores"||E.A%3==0?"EX_WallStores":Purpose=="Pump chamber"?"EX_PumpStation":"EX_PipeBank";
     if(Region!=1)Furnishing=((Region==3||Region==5)&&E.A%3==0)?"EX_WallStores":"REG_"+RegionalArt::FurnishingFor(Region,E.A/4);
     Mesh(Furnishing,P+Inward*65,FVector::OneVector,FRotator(0,Inward.Rotation().Yaw-90,0),"Authored");++Furniture;
    }
   }

   if(D.Rows[AY][AX]!='~'&&Torch++%9==0){FVector Inward=(FVector(AX*400,AY*400,0)-P).GetSafeNormal();Mesh("Brazier",P+Inward*65,FVector::OneVector,FRotator::ZeroRotator,"CorrodedIron");Light(P+Inward*130+FVector(0,0,175),10000,1100,FLinearColor(1,.54,.22));
    if(Region==1)Mesh("OCS_PipeOutlet",P+Inward*40+FVector(0,0,240),FVector(1.4),FRotator(0,Inward.Rotation().Yaw-90,0),"CorrodedIron");}
  }else if(E.Kind=="Pit"){
   if((D.Rows[AY][AX]=='~')!=(D.Rows[BY][BX]=='~')){Mesh("EX_BridgeRail",P,FVector::OneVector,Rot,"CorrodedIron");Box(P+FVector(0,0,125),FVector(200,16,125),Rot);
    Mesh("Cube",P+FVector(0,0,-420),FVector(4,.4,8.4),Rot,"ReservoirAshlar");}
  }else if(E.Kind!="Open"){
   Mesh("Arch",P,FVector::OneVector,Rot,"LimestoneCoping");int Z=FMath::Max(Height(E.A),Height(E.B));Mesh("Cube",P+FVector(0,0,(Z+520)*.5f),FVector(4,.46,(Z-520)/100.f),Rot);
   // The arch has a 3m opening; close its side margins up to the full ceiling.
   Box(P+Rot.RotateVector(FVector(-180,0,250)),FVector(20,25,250),Rot);Box(P+Rot.RotateVector(FVector(180,0,250)),FVector(20,25,250),Rot);
   Box(P+FVector(0,0,(Z+520)*.5f),FVector(200,25,(Z-520)*.5f),Rot);
   auto A=GetWorld()->SpawnActor<AStaticMeshActor>();A->SetMobility(EComponentMobility::Movable);auto C=A->GetStaticMeshComponent();FString Name=E.Kind=="Secret"?"Wall":E.Kind=="Bars"?"Grate":"Door";
   C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*("/Game/Game/Environment/SM_I_"+Name+".SM_I_"+Name)));FString Material=Region==1?SewerArt::MaterialFor(Name,E.Kind=="Secret"?"I_Sewer":"I_Rust"):RegionalArt::MaterialFor(Region,Name,E.Kind=="Secret"?"I_Limestone":"I_Rust");C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,*Material));C->SetCollisionEnabled(R.OpenDoors.Contains(E.DoorId)?ECollisionEnabled::NoCollision:ECollisionEnabled::QueryAndPhysics);C->SetCollisionResponseToAllChannels(ECR_Block);A->SetActorLocation(P+FVector(0,0,R.OpenDoors.Contains(E.DoorId)?520:0));A->SetActorRotation(Rot);Scenery.Add(A);DoorVisuals.Add({A,E.DoorId,P});
  }
 }
 for(auto& Pair:Runs){auto& Rn=Pair.Value;Rn.Along.Sort();for(int I=0;I<Rn.Along.Num();){int Start=Rn.Along[I],End=Start;while(++I<Rn.Along.Num()&&Rn.Along[I]==End+1)End=Rn.Along[I];
   FVector P=Rn.East?FVector(Rn.Plane*200,(Start+End)*200,Rn.Height/2.f):FVector((Start+End)*200,Rn.Plane*200,Rn.Height/2.f);FRotator Rot(0,Rn.East?90:0,0);float Length=(End-Start+1)*4;
   Mesh("Cube",P,FVector(Length,.46,Rn.Height/100.f),Rot);Box(P,FVector(Length*50,23,Rn.Height/2.f),Rot);
   Mesh("Cube",P+FVector(0,0,-Rn.Height/2.f+30),FVector(Length,.62,.6),Rot,"LimestoneCoping");
  }}
 // Full-height supports and overhead ribs establish smaller bays inside larger chambers.
 TSet<int> UsedSupports;
 for(auto V:R.Volumes){FVector P(V.X*400,V.Y*400,0);int Ceiling=Height(V.Y*W+V.X);
  Light(P+FVector(0,0,FMath::Min(360,Ceiling-180)),V.Purpose=="Great reservoir"?240000:110000,V.Purpose=="Great reservoir"?5200:3200,FLinearColor(.66f,.65f,.52f));
  if(V.Purpose=="Great reservoir"){
   for(int X:{-5,5})for(int Y:{-5,5}){Mesh("EX_ReservoirPier",P+FVector(X*400,Y*400,-900),FVector(1.5,1.5,(Ceiling+900)/965.f));Light(P+FVector(X*400,Y*400,-550),Region>=7?80000:23000,Region>=7?2600:1500,Region>=7?FLinearColor(1.f,.16f,.025f):FLinearColor(.30f,.46f,.36f));}
   // Stone crossbeams genuinely span the reservoir piers, above the playable bridges.
   for(int Y:{-5,5})Mesh("Cube",P+FVector(0,Y*400,Ceiling-50),FVector(42,1.2,1),FRotator::ZeroRotator,"LimestoneCoping");
  }else{
   for(int Y=V.Y-V.RadiusY+2;Y<V.Y+V.RadiusY-1;Y+=3)for(int X=V.X-V.RadiusX+2;X<V.X+V.RadiusX-1;X+=3){
    int C=Y*W+X;if(UsedSupports.Contains(C))continue;bool Clear=true;int Z=MAX_int32;
    for(int OY=0;OY<2;++OY)for(int OX=0;OX<2;++OX){int N=(Y+OY)*W+X+OX;TCHAR T=D.Rows[Y+OY][X+OX];if(T!='.'||R.Rooms.FindRef(N)!=V.Purpose)Clear=false;Z=FMath::Min(Z,Height(N));}
    if(!Clear)continue;UsedSupports.Add(C);FVector At(X*400+200,Y*400+200,0);
    Mesh("Pillar",At,FVector(1.4,1.4,Z/505.f),FRotator::ZeroRotator,"ReservoirAshlar");Box(At+FVector(0,0,Z*.5f),FVector(40,40,Z*.5f));
    Mesh("Cube",At+FVector(0,0,Z-32),FVector(3.5,1.2,.64),FRotator::ZeroRotator,"LimestoneCoping");++SupportCount;
   }
  }
 }
 // Shallow ribs articulate the lower ceiling; no small standalone arches in rooms.
 for(int Y=2;Y<D.Rows.Num()-2;Y+=3)for(int X=1;X<W-1;){
  int Start=X,Z=Height(Y*W+X);FString Purpose=R.Rooms.FindRef(Y*W+X);
  if(D.Rows[Y][X]=='#'||D.Rows[Y][X]=='~'||Purpose=="Great reservoir"){++X;continue;}
  while(X+1<W-1&&D.Rows[Y][X+1]!='#'&&D.Rows[Y][X+1]!='~'&&Height(Y*W+X+1)==Z&&R.Rooms.FindRef(Y*W+X+1)==Purpose)++X;
  Mesh("Cube",FVector((Start+X)*200,Y*400,Z-22),FVector((X-Start+1)*4,.6,.44),FRotator::ZeroRotator,"LimestoneCoping");++X;
 }
 UE_LOG(LogTemp,Display,TEXT("EXPANSION_ATMOSPHERE furnishing_clusters=%d full_height_supports=%d standalone_arches=0 ceilings=560..1400"),Furniture,SupportCount);
 FinishDungeonBatches(Batches);if(WorldLoading)PendingDressingRoot=Root;else{SewerArt::Decorate(this,Root);RegionalArt::Decorate(this,Root);}
 UE_LOG(LogTemp,Display,TEXT("EXPANSION_ARCHITECTURE batches=%d wall_groups=%d rooms=%d"),Batches.Num(),Runs.Num(),R.Volumes.Num());
}
