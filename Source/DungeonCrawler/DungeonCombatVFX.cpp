#include "DungeonCombatVFX.h"
#include "Rendering/DrawElements.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"

namespace {
struct FPixelRun { uint8 X,Y,Width,Color; };
struct FSpriteFrame { int32 First,Count; };
#include "DungeonCombatVFXData.inl"
FString CanonicalCue(const FString& Cue) {
 if(Cue==TEXT("guard"))return TEXT("defend");
 if(Cue==TEXT("miss")||Cue==TEXT("escape_fail"))return TEXT("dodge");
 return Cue;
}
}

float DungeonCombatVFX::Duration(const FString& Cue) {
 if(Cue==TEXT("fireball")||Cue==TEXT("ice_lance")||Cue==TEXT("shadow_bolt")||Cue==TEXT("soul_drain"))return .9f;
 if(Cue==TEXT("critical"))return .7f;
 return .8f;
}

int32 DungeonCombatVFX::Paint(const FString& Cue,const FGeometry& Geometry,FSlateWindowElementList& Elements,int32 Layer,FVector2D Center,float Size,float Progress) {
 if(Progress<0.f||Progress>=1.f||Size<=0.f)return Layer;
 const FString Id=CanonicalCue(Cue);int32 Row=INDEX_NONE;
 for(int32 I=0;I<UE_ARRAY_COUNT(SpriteCues);++I)if(Id==SpriteCues[I]){Row=I;break;}
 if(Row==INDEX_NONE)Row=0;
 const int32 Frame=FMath::Clamp(FMath::FloorToInt(Progress*8.f),0,7);
 const FSpriteFrame& Data=SpriteFrames[Row*8+Frame];
 TArray<FSlateVertex> Vertices;TArray<SlateIndex> Indices;
 Vertices.Reserve(Data.Count*4);Indices.Reserve(Data.Count*6);
 const float Unit=Size/96.f;
 const FVector2f Origin(Center.X-Size*.5f,Center.Y-Size*.5f);
 const float Fade=Progress>.87f?FMath::Clamp((1.f-Progress)/.13f,0.f,1.f):1.f;
 for(int32 I=Data.First;I<Data.First+Data.Count;++I){
  const auto& Run=SpriteRuns[I];FColor Color=SpritePalette[Run.Color];Color.A=uint8(Color.A*Fade);
  const float X=Origin.X+Run.X*Unit,Y=Origin.Y+Run.Y*Unit,W=Run.Width*Unit;
  const SlateIndex Base=Vertices.Num();
  const FVector2f Positions[]={FVector2f(X,Y),FVector2f(X+W,Y),FVector2f(X+W,Y+Unit),FVector2f(X,Y+Unit)};
  for(const auto& Position:Positions)Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(Geometry.GetAccumulatedRenderTransform(),Position,FVector2f(.5f,.5f),Color));
  Indices.Append({Base,SlateIndex(Base+1),SlateIndex(Base+2),Base,SlateIndex(Base+2),SlateIndex(Base+3)});
 }
 const auto Resource=FSlateApplication::Get().GetRenderer()->GetResourceHandle(*FCoreStyle::Get().GetBrush("WhiteBrush"));
 FSlateDrawElement::MakeCustomVerts(Elements,Layer++,Resource,Vertices,Indices,nullptr,0,0);
 return Layer;
}
