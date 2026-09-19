#include "DungeonStatusIcons.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Framework/Application/SlateApplication.h"

namespace DungeonStatusIcons
{
FLinearColor Color(const FString& S)
{
 if(S=="Poison")return FLinearColor(.58f,.9f,.27f);
 if(S=="Bleed"||S=="Bleeding"||S=="Disease"||S=="Plague"||S=="Illness")return FLinearColor(1.f,.2f,.18f);
 if(S=="Burn")return FLinearColor(1.f,.55f,.12f);
 if(S=="Chill"||S=="Freeze"||S=="Frozen"||S=="Slow")return FLinearColor(.38f,.82f,1.f);
 if(S=="Guard"||S=="Regen"||S=="Regeneration")return FLinearColor(.43f,.9f,.68f);
 if(S=="Stun"||S=="Mark")return FLinearColor(1.f,.86f,.3f);
 return FLinearColor(.8f,.57f,1.f);
}

void Draw(const FGeometry& G,FSlateWindowElementList& E,int32& Layer,const FString& S,FVector2D Position,float Size)
{
 const FLinearColor Tint=Color(S),Dark(.025f,.018f,.013f,.96f);
 auto Rect=[&](float X,float Y,float W,float H,FLinearColor C){FSlateDrawElement::MakeBox(E,Layer++,G.ToPaintGeometry(FVector2D(W*Size,H*Size),FSlateLayoutTransform(Position+FVector2D(X*Size,Y*Size))),FCoreStyle::Get().GetBrush("WhiteBrush"),ESlateDrawEffect::None,C);};
 auto Line=[&](std::initializer_list<FVector2D> Points,FLinearColor C,float Width=.07f){TArray<FVector2D> P;for(auto V:Points)P.Add(Position+V*Size);FSlateDrawElement::MakeLines(E,Layer++,G.ToPaintGeometry(),P,ESlateDrawEffect::None,C,true,FMath::Max(1.f,Size*Width));};
 auto Poly=[&](const TArray<FVector2D>& Points,FLinearColor C){
  if(Points.Num()<3)return;
  TArray<FSlateVertex> V;TArray<SlateIndex> I;TArray<int32> Remaining;
  double Area=0;
  for(int32 N=0;N<Points.Num();++N){const auto P=Points[N],Q=Points[(N+1)%Points.Num()];Area+=P.X*Q.Y-Q.X*P.Y;Remaining.Add(N);V.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),FVector2f(Position+P*Size),FVector2f(.5f,.5f),C.ToFColor(true)));}
  const double Winding=Area>=0?1.0:-1.0;
  auto Cross=[](FVector2D A,FVector2D B,FVector2D D){return (B.X-A.X)*(D.Y-A.Y)-(B.Y-A.Y)*(D.X-A.X);};
  // Ear clipping preserves the inward notches in flame, bolt and ghost silhouettes.
  while(Remaining.Num()>3){
   bool Clipped=false;
   for(int32 N=0;N<Remaining.Num();++N){
    const int32 A=Remaining[(N+Remaining.Num()-1)%Remaining.Num()],B=Remaining[N],D=Remaining[(N+1)%Remaining.Num()];
    if(Cross(Points[A],Points[B],Points[D])*Winding<=1.e-8)continue;
    bool Contains=false;
    for(int32 K:Remaining){if(K==A||K==B||K==D)continue;const auto P=Points[K];if(Cross(Points[A],Points[B],P)*Winding>=-1.e-8&&Cross(Points[B],Points[D],P)*Winding>=-1.e-8&&Cross(Points[D],Points[A],P)*Winding>=-1.e-8){Contains=true;break;}}
    if(Contains)continue;
    I.Append({SlateIndex(A),SlateIndex(B),SlateIndex(D)});Remaining.RemoveAt(N);Clipped=true;break;
   }
   if(!Clipped)return; // Do not emit malformed geometry if a future icon has an invalid path.
  }
  I.Append({SlateIndex(Remaining[0]),SlateIndex(Remaining[1]),SlateIndex(Remaining[2])});
  const auto R=FSlateApplication::Get().GetRenderer()->GetResourceHandle(*FCoreStyle::Get().GetBrush("WhiteBrush"));FSlateDrawElement::MakeCustomVerts(E,Layer++,R,V,I,nullptr,0,0);
 };
 auto Disc=[&](float X,float Y,float Radius,FLinearColor C){TArray<FVector2D> P;for(int32 N=0;N<20;++N){float A=2.f*PI*N/20.f;P.Add(FVector2D(X+FMath::Cos(A)*Radius,Y+FMath::Sin(A)*Radius));}Poly(P,C);};
 Rect(0,0,1,1,Dark);Rect(0,0,1,.045f,Tint);Rect(0,.955f,1,.045f,Tint);Rect(0,0,.045f,1,Tint);Rect(.955f,0,.045f,1,Tint);
 if(S=="Poison")
 {
  Line({{.23,.67},{.77,.87}},Tint,.085f);Line({{.23,.87},{.77,.67}},Tint,.085f);
  for(auto P:{FVector2D(.23,.67),FVector2D(.77,.87),FVector2D(.23,.87),FVector2D(.77,.67)})Disc(P.X,P.Y,.06f,Tint);
  Disc(.5f,.37f,.23f,Tint);Rect(.36f,.47f,.28f,.17f,Tint);Disc(.41f,.37f,.065f,Dark);Disc(.59f,.37f,.065f,Dark);Poly({{.5,.46},{.46,.52},{.54,.52}},Dark);Rect(.44f,.57f,.025f,.075f,Dark);Rect(.53f,.57f,.025f,.075f,Dark);
 }
 else if(S=="Bleed"||S=="Bleeding")
 {Poly({{.5,.12},{.26,.49},{.25,.65},{.35,.8},{.5,.86},{.65,.8},{.75,.65},{.74,.49}},Tint);Line({{.35,.55},{.34,.66},{.41,.74}},FLinearColor(1,.68f,.58f),.045f);}
 else if(S=="Disease"||S=="Plague"||S=="Illness")
 {Rect(.37f,.17f,.26f,.66f,Tint);Rect(.17f,.37f,.66f,.26f,Tint);}
 else if(S=="Burn")
 {Poly({{.5,.12},{.3,.49},{.24,.66},{.34,.82},{.52,.87},{.72,.76},{.79,.54},{.65,.33},{.6,.56}},Tint);Poly({{.5,.46},{.39,.7},{.5,.81},{.62,.71}},FLinearColor(1,.94f,.48f));}
 else if(S=="Chill"||S=="Freeze"||S=="Frozen"||S=="Slow")
 {Line({{.5,.14},{.5,.86}},Tint);Line({{.19,.31},{.81,.69}},Tint);Line({{.19,.69},{.81,.31}},Tint);Line({{.36,.21},{.5,.34},{.64,.21}},Tint,.045f);Line({{.36,.79},{.5,.66},{.64,.79}},Tint,.045f);}
 else if(S=="Stun")
 {Poly({{.52,.1},{.23,.55},{.46,.55},{.39,.91},{.79,.39},{.55,.39},{.65,.1}},Tint);}
 else if(S=="Guard")
 {Poly({{.2,.22},{.5,.13},{.8,.22},{.74,.66},{.5,.88},{.26,.66}},Tint);Line({{.5,.26},{.5,.72}},Dark,.07f);}
 else if(S=="Blind")
 {Line({{.15,.5},{.32,.32},{.68,.32},{.85,.5},{.68,.68},{.32,.68},{.15,.5}},Tint,.06f);Disc(.5,.5,.12,Tint);Line({{.21,.82},{.8,.18}},Dark,.15f);Line({{.21,.82},{.8,.18}},Tint,.075f);}
 else if(S=="Fear")
 {Disc(.5,.42,.27,Tint);Poly({{.23,.42},{.24,.85},{.39,.74},{.5,.86},{.62,.74},{.77,.85},{.77,.42}},Tint);Disc(.4,.42,.06,Dark);Disc(.6,.42,.06,Dark);Disc(.5,.61,.09,Dark);}
 else if(S=="Mark")
 {TArray<FVector2D> P;for(int N=0;N<=20;++N){float A=2*PI*N/20;P.Add(Position+FVector2D(.5+FMath::Cos(A)*.25,.5+FMath::Sin(A)*.25)*Size);}FSlateDrawElement::MakeLines(E,Layer++,G.ToPaintGeometry(),P,ESlateDrawEffect::None,Tint,true,Size*.055f);Line({{.5,.12},{.5,.35}},Tint);Line({{.5,.65},{.5,.88}},Tint);Line({{.12,.5},{.35,.5}},Tint);Line({{.65,.5},{.88,.5}},Tint);}
 else if(S=="Silence")
 {Line({{.25,.36},{.5,.25},{.75,.36},{.72,.65},{.28,.65},{.25,.36}},Tint);Line({{.2,.82},{.8,.18}},Tint,.1f);}
 else if(S=="Weakness"||S=="Weaken")
 {Line({{.5,.18},{.5,.77},{.26,.54}},Tint,.11f);Line({{.5,.77},{.74,.54}},Tint,.11f);}
 else if(S=="Regen"||S=="Regeneration")
 {Rect(.43,.2,.14,.6,Tint);Rect(.2,.43,.6,.14,Tint);}
 else // Curse, and a readable fallback for future hexes.
 {Poly({{.5,.14},{.79,.5},{.5,.86},{.21,.5}},Tint);Poly({{.5,.3},{.65,.5},{.5,.7},{.35,.5}},Dark);Disc(.5,.5,.07,Tint);}
}
}
