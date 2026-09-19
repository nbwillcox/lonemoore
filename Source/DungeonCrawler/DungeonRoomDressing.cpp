#include "DungeonRoomDressing.h"
#include "DungeonGame.h"
#include "DungeonRoomKit.h"
#include "DungeonRenderUtils.h"
#include "Engine/StaticMesh.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"

namespace {
struct FProp {FString Id,Mesh,Placement;int Region=0;FVector Min,Max;};
struct FAnchor {FVector Position,Inward;};
FVector VectorField(const TSharedPtr<FJsonObject>& O,const TCHAR* Field){
 const TArray<TSharedPtr<FJsonValue>>* V=nullptr;
 return O->TryGetArrayField(Field,V)&&V->Num()==3?FVector((*V)[0]->AsNumber(),(*V)[1]->AsNumber(),(*V)[2]->AsNumber()):FVector::ZeroVector;
}
TSharedPtr<FJsonObject> Read(const TCHAR* Name){FString Raw;TSharedPtr<FJsonObject> O;FFileHelper::LoadFileToString(Raw,*(FPaths::ProjectContentDir()/TEXT("Game/Data")/Name));FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Raw),O);return O;}
const TArray<FProp>& Props(){
 static TArray<FProp> Result;static bool Loaded=false;if(Loaded)return Result;Loaded=true;
 auto O=Read(TEXT("regional_dressing.json"));const TArray<TSharedPtr<FJsonValue>>* Values=nullptr;if(!O||!O->TryGetArrayField(TEXT("assets"),Values))return Result;
 for(auto V:*Values){auto P=V->AsObject();if(!P)continue;FProp A;P->TryGetStringField(TEXT("id"),A.Id);P->TryGetStringField(TEXT("mesh"),A.Mesh);P->TryGetStringField(TEXT("placement"),A.Placement);P->TryGetNumberField(TEXT("regionIndex"),A.Region);A.Min=VectorField(P,TEXT("minCm"));A.Max=VectorField(P,TEXT("maxCm"));if(A.Mesh.StartsWith(TEXT("/Game/RegionalDressing/"))&&A.Max.X>A.Min.X&&A.Max.Y>A.Min.Y)Result.Add(A);}
 return Result;
}
const TMap<FString,TArray<FAnchor>>& Anchors(){
 static TMap<FString,TArray<FAnchor>> Result;static bool Loaded=false;if(Loaded)return Result;Loaded=true;
 auto O=Read(TEXT("room_dressing_anchors.json"));const TArray<TSharedPtr<FJsonValue>>* Values=nullptr;if(!O||!O->TryGetArrayField(TEXT("templates"),Values))return Result;
 for(auto V:*Values){auto T=V->AsObject();if(!T)continue;FString Id;T->TryGetStringField(TEXT("id"),Id);const TArray<TSharedPtr<FJsonValue>>* List=nullptr;if(!T->TryGetArrayField(TEXT("anchors"),List))continue;for(auto A:*List){auto P=A->AsObject();if(P)Result.FindOrAdd(Id).Add({VectorField(P,TEXT("position")),VectorField(P,TEXT("inwardNormal")).GetSafeNormal()});}}
 return Result;
}
// Keep every traversable centre-to-centre lane clear, including doorway mouths.
// Bounding-box projection is deliberately conservative for irregular sculptures.
bool ClearLanes(const FRoomTemplate& Room,const FBox& Box){
 if(Box.Min.Z>245||Box.Max.Z<10)return true;
 const FBox Expanded=Box.ExpandBy(FVector(45,45,0));
 auto Overlaps=[&](FVector A,FVector B){return FMath::Max(A.X,B.X)>=Expanded.Min.X&&FMath::Min(A.X,B.X)<=Expanded.Max.X&&FMath::Max(A.Y,B.Y)>=Expanded.Min.Y&&FMath::Min(A.Y,B.Y)<=Expanded.Max.Y;};
 for(int Y=0;Y<7;++Y)for(int X=0;X<7;++X)if(Room.Rows[Y][X]=='.'){
  FVector A((X-3)*400,(Y-3)*400,0);if(Overlaps(A,A))return false;
  for(auto Step:{FIntPoint(1,0),FIntPoint(0,1)}){int NX=X+Step.X,NY=Y+Step.Y;if(NX<7&&NY<7&&Room.Rows[NY][NX]=='.'&&Overlaps(A,A+FVector(Step.X*400,Step.Y*400,0)))return false;}
  if((X==0||Y==0||X==6||Y==6)&&Overlaps(A,A+A.GetSafeNormal()*200))return false;
 }
 return true;
}
FString DecalPath(int Region,int Variant){
 const TCHAR* Codes[]={TEXT("Cathedral"),TEXT("Sewer"),TEXT("Catacombs"),TEXT("Warrens"),TEXT("Crypts"),TEXT("Fortress"),TEXT("Deep"),TEXT("Infernal"),TEXT("Hell")};
 const TCHAR* Names[][2]={{TEXT("WaxRun"),TEXT("SootFan")},{TEXT("RisingDamp"),TEXT("PipeJointLeak")},{TEXT("ChalkBloom"),TEXT("DustFall")},{TEXT("MudSplash"),TEXT("MossPatch")},{TEXT("ColdSeep"),TEXT("CandleWax")},{TEXT("RustRun"),TEXT("SootStain")},{TEXT("ClawScars"),TEXT("MineralBloom")},{TEXT("HeatScorch"),TEXT("RitualResidue")},{TEXT("ForgeScorch"),TEXT("SlagSpatter")}};
 const FString Id=Region==1?FString(Names[Region][Variant%2]):FString(Codes[Region])+TEXT("_")+Names[Region][Variant%2];
 return FString(Region==1?TEXT("/Game/OldCitySewers/Materials/MI_"):TEXT("/Game/RegionalArt/Materials/MI_"))+Id+TEXT(".MI_")+Id;
}
}
void RoomDressing::AddPreloadAssets(int32 Region,TArray<FSoftObjectPath>& Assets){
 for(const auto& P:Props())if(P.Region==Region)Assets.AddUnique(FSoftObjectPath(P.Mesh));
 for(int V=0;V<2;++V)Assets.AddUnique(FSoftObjectPath(DecalPath(FMath::Clamp(Region,0,8),V)));
}
void RoomDressing::Decorate(ADungeonController* Host,AActor* Root){
 auto M=Host->Model.Get();const auto& R=M->State->Floors[M->State->Floor];const int Region=FMath::Clamp(M->Floors[M->State->Floor].RegionIndex,0,8);
 TArray<const FProp*> Options;for(const auto& P:Props())if(P.Region==Region)Options.Add(&P);
 TMap<FString,UInstancedStaticMeshComponent*> Batches;int PropCount=0,DecalCount=0,Rejected=0;TSet<FString> Used;
 for(int Index=0;Index<R.RoomPlacements.Num();++Index){
  const auto& P=R.RoomPlacements[Index];const auto* T=DungeonRoomKit::Find(P.Id);const auto* Sites=Anchors().Find(P.Id);if(!T||!Sites||Sites->IsEmpty())continue;
  const FVector Origin(P.X*400,P.Y*400,0);const FRotator RoomRotation(0,P.Rotation*90,0);
  const int Limit=FMath::Min(4,Sites->Num());
  for(int J=0;J<Limit;++J){
   const auto& Site=(*Sites)[(J*2+Index)%Sites->Num()];
   if(!Options.IsEmpty()){
    // FBX reflects Blender's -Y front to Unreal +Y; point that face into the room.
    const auto& Prop=*Options[(Index+J)%Options.Num()];const FRotator LocalRotation(0,Site.Inward.Rotation().Yaw-90,0);
    bool Placed=false;
    for(float Scale:{1.f,.75f,.55f,.4f}){
     const FBox LocalBounds=FBox(Prop.Min,Prop.Max).TransformBy(FTransform(LocalRotation,FVector::ZeroVector,FVector(Scale)));
     const FVector E=LocalBounds.GetExtent();const float Depth=FMath::Abs(Site.Inward.X)*E.X+FMath::Abs(Site.Inward.Y)*E.Y;
     FVector Local=Site.Position+Site.Inward*(Depth+10)-LocalBounds.GetCenter();
     // This pack's wall category means floor-standing beside a wall, not hanging.
     Local.Z=2.f-LocalBounds.Min.Z;
     const FBox Positioned=LocalBounds.ShiftBy(Local);
     if(!ClearLanes(*T,Positioned))continue;
     auto C=Batches.FindRef(Prop.Mesh);if(!C){auto Mesh=LoadObject<UStaticMesh>(nullptr,*Prop.Mesh);if(!Mesh){UE_LOG(LogTemp,Error,TEXT("ROOM_DRESSING_MISSING %s"),*Prop.Mesh);break;}C=DungeonBatch(Root,*FString::Printf(TEXT("Dressing_%d"),Batches.Num()));C->SetStaticMesh(Mesh);C->SetupAttachment(Root->GetRootComponent());C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCastShadow(Prop.Placement!=TEXT("patch"));C->SetCullDistances(6000,8500);Batches.Add(Prop.Mesh,C);}
     C->AddInstance(FTransform(RoomRotation+LocalRotation,Origin+RoomRotation.RotateVector(Local),FVector(Scale)));++PropCount;Used.Add(Prop.Id);Placed=true;break;
    }
    if(!Placed)++Rejected;
   }
   // Existing high-resolution grime is projected onto measured authored walls,
   // never guessed from the square navigation boundary of a curved room.
   if(J%2==0){auto Mat=LoadObject<UMaterialInterface>(nullptr,*DecalPath(Region,Index+J));if(!Mat)continue;
    auto C=NewObject<UDecalComponent>(Root,*FString::Printf(TEXT("RoomGrime_%d"),DecalCount));C->SetupAttachment(Root->GetRootComponent());C->SetDecalMaterial(Mat);C->DecalSize=FVector(18,110,125);C->SetFadeScreenSize(.012f);C->RegisterComponent();C->SetWorldLocationAndRotation(Origin+RoomRotation.RotateVector(Site.Position+Site.Inward*4),RoomRotation.RotateVector(-Site.Inward).Rotation());++DecalCount;
   }
  }
 }
 FinishDungeonBatches(Batches);
 UE_LOG(LogTemp,Display,TEXT("ROOM_DRESSING floor=%d region=%d props=%d types=%d decals=%d batches=%d rejected_lane_overlap=%d"),M->State->Floor,Region,PropCount,Used.Num(),DecalCount,Batches.Num(),Rejected);
}
