#include "DungeonGame.h"
#include "DungeonSewerArt.h"
#include "DungeonRegionalArt.h"
#include "DungeonRoomKit.h"
#include "DungeonRoomDressing.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "ShaderPipelineCache.h"

void ADungeonController::BeginWorldPreparation(){
 WorldLoading=true;WorldBuildStage=1;WorldWarmFrames=0;WorldLoadProgress=.05f;WorldLoadStarted=FPlatformTime::Seconds();PendingDressingRoot.Reset();
 TArray<FSoftObjectPath> Assets;FString Raw;TSharedPtr<FJsonObject> Manifest;
 if(FFileHelper::LoadFileToString(Raw,*(FPaths::ProjectContentDir()/TEXT("Game/Data/scene_streaming.json")))&&FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Raw),Manifest)){
  auto Add=[&](const TArray<TSharedPtr<FJsonValue>>& Entries){for(const auto& V:Entries)Assets.AddUnique(FSoftObjectPath(V->AsString()));};
  Add(Manifest->GetArrayField(TEXT("common")));const auto& Regions=Manifest->GetArrayField(TEXT("regions"));int Region=Model->Floors[Model->State->Floor].RegionIndex;if(Regions.IsValidIndex(Region))Add(Regions[Region]->AsArray());
 }
 Assets.Add(FSoftObjectPath(FString::Printf(TEXT("/Game/CampaignExpansion/Meshes/SM_Stair_%02d.SM_Stair_%02d"),Model->State->Floor,Model->State->Floor)));
 if(Model->State->Floors[Model->State->Floor].ContentVersion==4){
  RoomDressing::AddPreloadAssets(Model->Floors[Model->State->Floor].RegionIndex,Assets);
  for(const auto& P:Model->State->Floors[Model->State->Floor].RoomPlacements)if(const auto* T=DungeonRoomKit::Find(P.Id))Assets.AddUnique(FSoftObjectPath(T->Mesh));
  Assets.AddUnique(FSoftObjectPath(TEXT("/Game/RoomKit/Meshes/SM_RK_portal_cap.SM_RK_portal_cap")));
  const TCHAR* Codes[]={TEXT("Cathedral"),TEXT("Sewer"),TEXT("Catacombs"),TEXT("Warrens"),TEXT("Crypts"),TEXT("Fortress"),TEXT("Deep"),TEXT("Infernal"),TEXT("Hell")};
  const TCHAR* Roles[]={TEXT("Wall"),TEXT("Floor"),TEXT("Vault"),TEXT("Trim"),TEXT("Iron")};
  for(auto SurfaceRole:Roles){FString Name=FString("MI_RK_")+Codes[FMath::Clamp(Model->Floors[Model->State->Floor].RegionIndex,0,8)]+"_"+SurfaceRole;Assets.AddUnique(FSoftObjectPath("/Game/RoomKit/Materials/"+Name+"."+Name));}
 }
 ScenePreloadHandle=UAssetManager::GetStreamableManager().RequestAsyncLoad(Assets,FStreamableDelegate(),FStreamableManager::AsyncLoadHighPriority);
 UE_LOG(LogTemp,Display,TEXT("DUNGEON_STREAM_BEGIN floor=%d assets=%d"),Model->State->Floor,Assets.Num());
}

void ADungeonController::TickWorldPreparation(){
 if(WorldBuildStage==1){
  WorldLoadProgress=.05f+.60f*(ScenePreloadHandle?ScenePreloadHandle->GetProgress():1.f);
  if(ScenePreloadHandle&&!ScenePreloadHandle->HasLoadCompleted())return;
  WorldBuildStage=2;return; // Let the loading artwork paint before scene creation.
 }
 if(WorldBuildStage==2){RebuildWorld();Model->WorldDirty=false;UpdateView(1.f);WorldBuildStage=3;WorldLoadProgress=.80f;return;}
 if(WorldBuildStage==3){
  if(auto Root=PendingDressingRoot.Get()){SewerArt::Decorate(this,Root);RegionalArt::Decorate(this,Root);}
  PendingDressingRoot.Reset();WorldBuildStage=4;WorldLoadProgress=.94f;return;
 }
 // Give registered geometry/PSOs a few rendered frames before accepting input.
 if(++WorldWarmFrames<3||(FShaderPipelineCache::NumPrecompilesRemaining()>0&&FPlatformTime::Seconds()-WorldLoadStarted<3.0))return;
 WorldLoading=false;WorldLoadProgress=1;WorldBuildStage=0;
 UE_LOG(LogTemp,Display,TEXT("DUNGEON_STREAM_READY floor=%d seconds=%.3f"),Model->State->Floor,FPlatformTime::Seconds()-WorldLoadStarted);
}
