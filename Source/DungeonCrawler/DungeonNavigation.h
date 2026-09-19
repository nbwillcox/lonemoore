#pragma once
#include "CoreMinimal.h"
// North is decreasing grid Y in both views. World metres never enter UI transforms.
struct FDungeonMapTransform {
 FVector2D Origin;
 float Scale=32;
 int32 Width=19;
 FIntRect VisibleCells(FVector2D TopLeft,FVector2D Size,int32 Height)const{
  const float S=FMath::Max(Scale,.01f);const FVector2D A=(TopLeft-Origin)/S,B=(TopLeft+Size-Origin)/S;
  return FIntRect(FMath::Clamp(FMath::FloorToInt(A.X)-1,0,Width),FMath::Clamp(FMath::FloorToInt(A.Y)-1,0,Height),FMath::Clamp(FMath::CeilToInt(B.X)+1,0,Width),FMath::Clamp(FMath::CeilToInt(B.Y)+1,0,Height));
 }
 FVector2D CellCenter(int32 Cell)const{return Origin+FVector2D(Cell%Width+.5f,Cell/Width+.5f)*Scale;}
 int32 CellAt(FVector2D P)const{P=(P-Origin)/Scale;int X=FMath::FloorToInt(P.X),Y=FMath::FloorToInt(P.Y);return X>=0&&X<Width&&Y>=0?Y*Width+X:-1;}
};
