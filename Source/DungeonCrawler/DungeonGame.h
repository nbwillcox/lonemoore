#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "DungeonModel.h"
#include "DungeonGame.generated.h"
class ACameraActor; class UInstancedStaticMeshComponent; class UAudioComponent; class USoundBase;
struct FStreamableHandle;

UCLASS()
class ADungeonGameMode : public AGameModeBase {
 GENERATED_BODY()
public: ADungeonGameMode();
};
UCLASS()
class ADungeonController : public APlayerController {
 GENERATED_BODY()
public:
 UPROPERTY() TObjectPtr<UDungeonModel> Model;
 UPROPERTY() TObjectPtr<class UDungeonWidget> Interface;
 UPROPERTY() TMap<FString,TObjectPtr<UTexture2D>> Textures;
 UPROPERTY() TMap<FString,TObjectPtr<USoundBase>> CueSounds;
 TMap<FString,float> SpriteBottomPadding;
 UPROPERTY() TObjectPtr<ACameraActor> DungeonCamera;
 UPROPERTY() TArray<TObjectPtr<AActor>> Scenery;
 UPROPERTY() TArray<TObjectPtr<AActor>> Architecture;
 TWeakObjectPtr<UDungeonSave> BuiltState;
 int32 BuiltFloor=-1,BuiltSeed=0,BuiltVersion=-1,ArchitectureBuilds=0;
 bool ReuseArchitecture() const;
 bool WorldLoading=false;
 float WorldLoadProgress=0;
 int32 WorldBuildStage=0,WorldWarmFrames=0;
 double WorldLoadStarted=0;
 TSharedPtr<FStreamableHandle> ScenePreloadHandle;
 TWeakObjectPtr<AActor> PendingDressingRoot;
 void BeginWorldPreparation();
 void TickWorldPreparation();
 UPROPERTY() TArray<TObjectPtr<AActor>> Billboards;
 UPROPERTY() TObjectPtr<UAudioComponent> Music;
 FVector CameraFrom,CameraTo;FRotator RotationFrom,RotationTo;float Tween=1;
 int32 LastX=-1,LastY=-1,LastFacing=-1;
 int32 UIPage=0;
 int32 MapFloor=-1;
 float MapZoom=32.f,DetectionClock=0,AtmosphereClock=0;
 FVector2D MapPan=FVector2D::ZeroVector;
 struct FDoorVisual { TWeakObjectPtr<class AStaticMeshActor> Actor; FString Id; FVector Closed; float LastAlpha=-1.f; };
 TArray<FDoorVisual> DoorVisuals;
 TArray<TWeakObjectPtr<class APointLight>> Flames;
 TWeakObjectPtr<class APointLight> PlayerTorch,PlayerTorchFill;
 struct FWorldEnemyVisual { TWeakObjectPtr<AActor> Actor; TWeakObjectPtr<class UMaterialInstanceDynamic> Material; };
 TArray<FWorldEnemyVisual> WorldEnemyVisuals;
 UPROPERTY() TObjectPtr<class UInstancedStaticMeshComponent> Dust;
 TArray<FVector> DustOrigins;
 struct FHoverKey { TWeakObjectPtr<AActor> Actor; FVector Origin; float Phase; };
 TArray<FHoverKey> HoverKeys;
 float KeyHoverClock=0;
 FVector LastVisualCamera=FVector(1.e20);
 bool LastVisualCombat=false,LastTorchVisible=true;
 void TickSharedPropReview(float Delta);
 void TickRegionalArtReview(float Delta);
 void StartRegionalArtPlaytest();
 void BuildArchitecture();
 void BuildExpandedArchitecture();
 void BuildModularArchitecture();
 void StartRoomKitPlaytest();
 void TickRoomKitReview(float Delta);
 void StartExpansionPlaytest();
 void TickExpansionReview(float Delta);
 void TickFineTuneReview(float Delta);
 void TickAdventurePolishReview(float Delta);
 void TickCampaignExpansionReview(float Delta);
 void TickAtmosphere(float Delta);
 void TickIntegrityReview(float Delta);
 void TickSewerArtReview(float Delta);
 void StartSewerArtPlaytest();
 void TickInventoryStatusReview(float Delta);
 void TickAccessoryReview(float Delta);
 void TickCombatFXReview(float Delta);
 void TickStorefrontReview(float Delta);
 int32 StorefrontFailures=0,StorefrontGold=0,StorefrontQuality=0;
 FString StorefrontAction;
 FString PendingCommand,ConfirmText,BindingCapture;
 int32 SettingsPage=0,HistoryPage=0;
 bool SellMode=false,ShowHistory=false,StartupCaptured=false;
 float MasterVolume=1.f,MusicVolume=.5f,EffectsVolume=.75f,Brightness=1.f,StartupTime=0,PageTurn=0;
 TMap<FString,FKey> Bindings;
 TArray<FString> StartupAssets;
 int32 LoadedAssets=0;
 int32 LoadedCues=0;
 void SavePreferences();
 void LoadPreferences();
 bool Pressed(const FString& Action) const;

 FString MusicId;
 FString SettingsReturn="Menu",LoadReturn="Menu";
 bool CreditsFromEnding=false;
 bool ReviewMode=false;
 float ReviewTime=0;
 int32 ReviewStep=0;
 FString PendingCapture;
 float CaptureDelay=0;
 virtual void BeginPlay() override;
 virtual void PlayerTick(float Delta) override;
 void Command(const FString& Id);
 void RebuildWorld();
 void UpdateView(float Delta);
 void PlayCue(const FString& Id);
 void TickReview(float Delta);
 bool ReviewClick(const FString& Id,bool Double=false);
 UTexture2D* Texture(const FString& Id) const;
};
struct FUIHit {FBox2D Bounds;FString Id;};
UCLASS()
class UDungeonWidget : public UUserWidget {
 GENERATED_BODY()
public:
 UPROPERTY() TObjectPtr<ADungeonController> Host;
 mutable TArray<FUIHit> Hits;
 mutable TMap<FString,FSlateBrush> ImageBrushes;
 FVector2D Mouse=FVector2D(-100,-100);
 mutable int32 ShopSelection=0;
 mutable FString ShopContext;
 mutable FString ShopDetailKey;
 mutable float ShopDetailOffset=0,ShopDetailMax=0;
 int32 NameCaret=0;
 bool NameSelectAll=false;
 void InsertNameText(const FString& Text);
 virtual FReply NativeOnKeyChar(const FGeometry& G,const FCharacterEvent& E) override;
 virtual FReply NativeOnKeyDown(const FGeometry& G,const FKeyEvent& E) override;
 virtual int32 NativePaint(const FPaintArgs& Args,const FGeometry& AllottedGeometry,const FSlateRect& MyCullingRect,FSlateWindowElementList& OutDrawElements,int32 LayerId,const FWidgetStyle& InWidgetStyle,bool bParentEnabled) const override;
 virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& G,const FPointerEvent& E) override;
 virtual FReply NativeOnMouseButtonDown(const FGeometry& G,const FPointerEvent& E) override;
 virtual FReply NativeOnMouseMove(const FGeometry& G,const FPointerEvent& E) override;
 virtual FReply NativeOnMouseWheel(const FGeometry& G,const FPointerEvent& E) override;
 virtual FReply NativeOnMouseButtonUp(const FGeometry& G,const FPointerEvent& E) override;
 bool MapDragging=false;
};
