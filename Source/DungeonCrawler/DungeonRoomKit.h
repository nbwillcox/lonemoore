#pragma once
#include "CoreMinimal.h"

// The Blender manifest is the common contract for geometry, navigation and sockets.
struct FRoomMarker {
 FVector Position=FVector::ZeroVector;
 float Yaw=0;
 FString Kind;
 int32 Variant=0;
};
struct FRoomTemplate {
 FString Id,Name,Mesh;
 TArray<FString> Rows;
 int32 SocketMask=0;
 int32 CeilingCm=600;
 TArray<FRoomMarker> Lights,Props;
};
namespace DungeonRoomKit {
 constexpr int32 TileCount=7;
 constexpr int32 HalfTiles=3;
 constexpr float CellCm=400.f;
 const TArray<FRoomTemplate>& Templates();
 const FRoomTemplate* Find(const FString& Id);
 bool Validate(FString& Error);
 FIntPoint RotateCell(FIntPoint Offset,int32 QuarterTurns);
 int32 RotateMask(int32 Mask,int32 QuarterTurns);
 TCHAR Tile(const FRoomTemplate& Room,int32 X,int32 Y,int32 QuarterTurns);
}
