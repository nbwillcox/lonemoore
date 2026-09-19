#include "DungeonCombatVFX.h"
#include "DungeonStatusIcons.h"
#include "DungeonGame.h"
#include "DungeonNavigation.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Engine/Texture2D.h"
#include "GameFramework/GameUserSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Layout/Clipping.h"
#include "HAL/PlatformApplicationMisc.h"

namespace {
const FLinearColor Ink(.007f,.006f,.005f,.98f),Gold(.72f,.54f,.28f,1),White(.91f,.88f,.79f,1),Muted(.57f,.59f,.59f,1),Red(.55f,.12f,.1f,1),Blue(.12f,.37f,.56f,1);
const FString Qualities[]={"Worn","Standard","Fine","Masterwork","Legendary"};
struct FPainter {
 const FGeometry& G;FSlateWindowElementList& E;mutable int32 L;const UDungeonWidget* W;
 void Rect(float X,float Y,float Width,float Height,FLinearColor C)const{FSlateDrawElement::MakeBox(E,L++,G.ToPaintGeometry(FVector2D(Width,Height),FSlateLayoutTransform(FVector2D(X,Y))),FCoreStyle::Get().GetBrush("WhiteBrush"),ESlateDrawEffect::None,C);}
 void Text(FString T,float X,float Y,int32 Size=20,FLinearColor C=White)const{FSlateDrawElement::MakeText(E,L++,G.ToPaintGeometry(FVector2D(1500,Size+8),FSlateLayoutTransform(FVector2D(X,Y))),T,FCoreStyle::GetDefaultFontStyle("Regular",Size),ESlateDrawEffect::None,C);}
 void StoneText(FString T,float X,float Y,float Width,float Height,int32 Size=12,FLinearColor C=Gold)const{T=T.ToUpper();auto Font=FCoreStyle::GetDefaultFontStyle("Bold",Size);auto Extent=FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(T,Font);while(Extent.X>Width&&Size>8){Font.Size=--Size;Extent=FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(T,Font);}FVector2D At(X+(Width-Extent.X)/2,Y+(Height-Extent.Y)/2);auto Draw=[&](FVector2D Offset,FLinearColor Tint){FSlateDrawElement::MakeText(E,L++,G.ToPaintGeometry(Extent,FSlateLayoutTransform(At+Offset)),T,Font,ESlateDrawEffect::None,Tint);};Draw(FVector2D(0,1),FLinearColor(.42f,.39f,.32f));Draw(FVector2D(0,-1),FLinearColor::Black);Draw(FVector2D::ZeroVector,FLinearColor(.22f,.20f,.16f));}
 void Lines(FString T,float X,float Y,int32 Size,int32 Chars,FLinearColor C=White)const{TArray<FString> Words;T.ParseIntoArrayWS(Words);FString Line;for(auto Word:Words){if(Line.Len()+Word.Len()>Chars){Text(Line,X,Y,Size,C);Y+=Size*1.55f;Line.Empty();}Line+=Word+" ";}if(!Line.IsEmpty())Text(Line,X,Y,Size,C);}
 void Border(float X,float Y,float Width,float Height,FLinearColor C=Gold)const{Rect(X,Y,Width,1,C);Rect(X,Y+Height-1,Width,1,C);Rect(X,Y,1,Height,C);Rect(X+Width-1,Y,1,Height,C);}
 // Word wrapping uses measured glyph widths, so full shop details stay inside the side panel.
 float Wrap(const FString& Value,float X,float Y,float Width,int32 Size=17,FLinearColor Color=White)const{
  TArray<FString> Paragraphs;Value.ParseIntoArray(Paragraphs,TEXT("\n"),false);
  const auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();const auto Font=FCoreStyle::GetDefaultFontStyle("Regular",Size);
  for(const FString& Paragraph:Paragraphs){TArray<FString> Words;Paragraph.ParseIntoArrayWS(Words);FString Line;
   for(const FString& Word:Words){const FString Next=Line.IsEmpty()?Word:Line+" "+Word;if(!Line.IsEmpty()&&Measure->Measure(Next,Font).X>Width){Text(Line,X,Y,Size,Color);Y+=Size*1.5f;Line=Word;}else Line=Next;}
   if(!Line.IsEmpty())Text(Line,X,Y,Size,Color);Y+=Size*1.5f;
  }return Y;
 }
 void CenterText(const FString& Label,float X,float Y,float Width,float Height,int32 Size,FLinearColor Color)const{
  const auto Extent=FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Label,FCoreStyle::GetDefaultFontStyle("Regular",Size));
  Text(Label,X+(Width-Extent.X)/2,Y+(Height-Extent.Y)/2,Size,Color);
 }
 void SmokeBorder(float X,float Y,float Width,float Height,float Time)const{
  TArray<FSlateVertex> Vertices;TArray<SlateIndex> Indices;
  constexpr int32 Segments=24,Rings=5;
  // Soft, layered black plumes drift along the paper without obscuring its controls.
  for(int32 Side=0;Side<4;++Side)for(int32 Puff=0;Puff<22;++Puff){
   const float Seed=Side*27.31f+Puff*5.17f;
   const float Along=FMath::Frac(Puff/22.f+Time*.012f);
   const float Drift=FMath::Sin(Time*.65f+Seed)*14.f;
   const FVector2f Center=Side<2?FVector2f(X+Along*Width,Y+(Side==0?0:Height)+Drift):FVector2f(X+(Side==2?0:Width)+Drift,Y+Along*Height);
   const float Radius=38.f+14.f*FMath::Sin(Seed+Time*.4f);
   const SlateIndex Start=Vertices.Num();
   for(int32 Ring=0;Ring<=Rings;++Ring)for(int32 Segment=0;Segment<Segments;++Segment){
    const float R=float(Ring)/Rings,Angle=2.f*PI*Segment/Segments;
    const float Billow=1.f+.16f*FMath::Sin(Angle*3.f+Seed+Time*.55f);
    const FVector2f Position=Center+FVector2f(FMath::Cos(Angle)*Radius*Billow*R,FMath::Sin(Angle)*Radius*Billow*R*1.3f);
    const uint8 Alpha=uint8(145.f*FMath::Square(1.f-R));
    Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),Position,FVector2f(.5f,.5f),FColor(0,0,0,Alpha)));
   }
   for(int32 Ring=0;Ring<Rings;++Ring)for(int32 Segment=0;Segment<Segments;++Segment){
    const SlateIndex A=Start+Ring*Segments+Segment,B=Start+Ring*Segments+(Segment+1)%Segments,C=A+Segments,D=B+Segments;
    Indices.Append({A,C,B,B,C,D});
   }
  }
  const auto Resource=FSlateApplication::Get().GetRenderer()->GetResourceHandle(*FCoreStyle::Get().GetBrush("WhiteBrush"));
  FSlateDrawElement::MakeCustomVerts(E,L++,Resource,Vertices,Indices,nullptr,0,0);
 }
 // Pixel regions reference the supplied 1448 x 1086 atlas; no source image is altered.
 void Art(float SX,float SY,float SW,float SH,float X,float Y,float Width,float Height,FLinearColor Tint=FLinearColor::White)const{
  auto T=W->Host->Texture("ui_main");if(!T)return;FString Key=FString::Printf(TEXT("ui_%g_%g_%g_%g"),SX,SY,SW,SH);auto& B=W->ImageBrushes.FindOrAdd(Key);B.SetResourceObject(T);B.DrawAs=ESlateBrushDrawType::Image;B.SetUVRegion(FBox2f(FVector2f(SX/1448.f,SY/1086.f),FVector2f((SX+SW)/1448.f,(SY+SH)/1086.f)));FSlateDrawElement::MakeBox(E,L++,G.ToPaintGeometry(FVector2D(Width,Height),FSlateLayoutTransform(FVector2D(X,Y))),&B,ESlateDrawEffect::None,Tint);
 }
 // Repeat pixels at one uniform scale; crop the final repeat instead of stretching it.
 void TileArt(float SX,float SY,float SW,float SH,float X,float Y,float Width,float Height,float Scale)const{
  if(SW<=0||SH<=0||Width<=0||Height<=0||Scale<=0)return;
  const float TW=SW*Scale,TH=SH*Scale;
  for(float V=0;V<Height-.01f;V+=TH)for(float U=0;U<Width-.01f;U+=TW){float DW=FMath::Min(TW,Width-U),DH=FMath::Min(TH,Height-V);Art(SX,SY,DW/Scale,DH/Scale,X+U,Y+V,DW,DH);}
 }
 void Skin(float SX,float SY,float SW,float SH,float CX,float CY,float Scale,float X,float Y,float Width,float Height,bool Fill=true)const{
  Scale=FMath::Min(Scale,FMath::Min(Width/(2*CX),Height/(2*CY)));const float DX=CX*Scale,DY=CY*Scale;
  for(int32 R=0;R<3;++R)for(int32 Q=0;Q<3;++Q){if(!Fill&&R==1&&Q==1)continue;
   float AX=Q==0?SX:Q==1?SX+CX:SX+SW-CX,AY=R==0?SY:R==1?SY+CY:SY+SH-CY;
   TileArt(AX,AY,Q==1?SW-2*CX:CX,R==1?SH-2*CY:CY,Q==0?X:Q==1?X+DX:X+Width-DX,R==0?Y:R==1?Y+DY:Y+Height-DY,Q==1?Width-2*DX:DX,R==1?Height-2*DY:DY,Scale);
  }
 }
 void ArtFit(float SX,float SY,float SW,float SH,float X,float Y,float Width,float Height)const{float Scale=FMath::Min(Width/SW,Height/SH);Art(SX,SY,SW,SH,X+(Width-SW*Scale)/2,Y+(Height-SH*Scale)/2,SW*Scale,SH*Scale);}
 void Frame(float X,float Y,float Width,float Height)const{Skin(550,24,480,225,50,50,.38f,X,Y,Width,Height,false);}
 void Paper(float X,float Y,float Width,float Height)const{TileArt(1110,85,180,95,X+14,Y+14,Width-28,Height-28,.8f);Skin(1050,26,382,215,50,50,.3f,X,Y,Width,Height,false);}
 FString Fit(FString Value,float Width,int32 Size)const{auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();auto Font=FCoreStyle::GetDefaultFontStyle("Regular",Size);if(Measure->Measure(Value,Font).X<=Width)return Value;while(Value.Len()>0&&Measure->Measure(Value+"...",Font).X>Width)Value.LeftChopInline(1);return Value+"...";}
 void TextBox(FString Value,float X,float Y,float Width,float Height,int32 Size=18,FLinearColor Color=White)const{
  auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();while(Size>9&&(Measure->Measure(Value,FCoreStyle::GetDefaultFontStyle("Regular",Size)).X>Width||Measure->Measure(Value,FCoreStyle::GetDefaultFontStyle("Regular",Size)).Y>Height))--Size;
  Value=Fit(Value,Width,Size);float TextHeight=Measure->Measure(Value,FCoreStyle::GetDefaultFontStyle("Regular",Size)).Y;
  E.PushClip(FSlateClippingZone(G.ToPaintGeometry(FVector2D(Width,Height),FSlateLayoutTransform(FVector2D(X,Y)))));Text(Value,X,Y+FMath::Max(0.f,(Height-TextHeight)/2),Size,Color);E.PopClip();
 }
 void Line(FVector2D A,FVector2D B,FLinearColor Color,float Thickness=2)const{TArray<FVector2D> Points{A,B};FSlateDrawElement::MakeLines(E,L++,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,Color,true,Thickness);}
 void Navigation(int Floor,FVector2D Center,FVector2D Size,float Zoom,FVector2D Focus,bool Radar)const{
  auto M=W->Host->Model.Get();auto S=M->State;const auto& R=S->Floors[Floor];int Width=M->Floors[Floor].Rows[0].Len();FDungeonMapTransform T{Center-Focus*Zoom,Zoom,Width};FVector2D TL=Center-Size/2;
  Rect(TL.X,TL.Y,Size.X,Size.Y,FLinearColor(.016,.020,.018,.98));E.PushClip(FSlateClippingZone(G.ToPaintGeometry(Size,FSlateLayoutTransform(TL))));
  const FIntRect Cells=T.VisibleCells(TL,Size,M->Floors[Floor].Rows.Num());
  for(int Y=Cells.Min.Y;Y<Cells.Max.Y;++Y)for(int X=Cells.Min.X;X<Cells.Max.X;++X){int C=Y*Width+X;const auto KnownTile=R.MemoryTiles.Find(C);if(!KnownTile)continue;FVector2D Pos=T.CellCenter(C);bool Live=Floor==S->Floor&&M->Visible.Contains(C);const FString& Tile=*KnownTile;if(Tile=="#")continue;
   if(Tile=="~"){Rect(Pos.X-Zoom/2,Pos.Y-Zoom/2,Zoom,Zoom,Live?FLinearColor(.04,.13,.14):FLinearColor(.025,.05,.05));continue;}
   Rect(Pos.X-Zoom/2+1,Pos.Y-Zoom/2+1,Zoom-2,Zoom-2,Live?FLinearColor(.30,.31,.25):FLinearColor(.11,.12,.10));
   FString Icon=Tile==">"||Tile=="A"?">":Tile=="S"||Tile=="V"?"+":Tile=="L"?"L":Tile=="K"?"k":Tile=="C"||Tile=="$"?"o":Tile=="!"?"i":"";
   if(!Icon.IsEmpty())TextBox(Icon,Pos.X-Zoom*.18f,Pos.Y-Zoom*.45f,Zoom*.8f,Zoom*.9f,FMath::Clamp(int(Zoom*.6f),8,24),Live?Gold:Muted);
   if(R.Markers.Contains(C)){Line(Pos+FVector2D(-5,0),Pos+FVector2D(0,-6),Blue);Line(Pos+FVector2D(0,-6),Pos+FVector2D(5,0),Blue);Line(Pos+FVector2D(5,0),Pos+FVector2D(0,6),Blue);Line(Pos+FVector2D(0,6),Pos+FVector2D(-5,0),Blue);}
  }
  auto DrawEdge=[&](int From,int To){
   const uint64 Key=(uint64(uint32(FMath::Min(From,To)))<<32)|uint32(FMath::Max(From,To));
   if(!M->BoundaryIndices.IsValidIndex(Floor))return;const int* Index=M->BoundaryIndices[Floor].Find(Key);if(!Index||!R.Boundaries.IsValidIndex(*Index))return;
   const auto& Edge=R.Boundaries[*Index];const FString* Known=R.MemoryEdges.Find(Edge.Id);if(!Known||*Known=="Open"||Edge.B<0)return;
   FVector2D A=T.CellCenter(Edge.A),B=T.CellCenter(Edge.B),Mid=(A+B)/2;FVector2D Span=FMath::Abs(A.X-B.X)>1?FVector2D(0,Zoom/2):FVector2D(Zoom/2,0);Line(Mid-Span,Mid+Span,*Known=="Pit"?Blue:*Known=="Wall"?Muted:Gold,*Known=="Wall"?2:4);if(*Known=="Bars")Line(Mid-Span*.5f+FVector2D(2,2),Mid+Span*.5f+FVector2D(2,2),White,1);
  };
  for(int Y=Cells.Min.Y;Y<Cells.Max.Y;++Y)for(int X=Cells.Min.X;X<Cells.Max.X;++X){int C=Y*Width+X;if(X+1<Width)DrawEdge(C,C+1);if(Y+1<M->Floors[Floor].Rows.Num())DrawEdge(C,C+Width);}
  if(Floor==S->Floor){for(int C:M->DetectedEnemies){FVector2D Pos=T.CellCenter(C);Line(Pos+FVector2D(-4,-4),Pos+FVector2D(4,4),Red,3);Line(Pos+FVector2D(4,-4),Pos+FVector2D(-4,4),Red,3);}
   FVector2D Pos=T.CellCenter(M->Cell(S->X,S->Y));const FVector2D Directions[]={FVector2D(0,-1),FVector2D(1,0),FVector2D(0,1),FVector2D(-1,0)};FVector2D Dir=Directions[S->Facing],Side(-Dir.Y,Dir.X);Line(Pos+Dir*8,Pos-Dir*6+Side*5,White,3);Line(Pos+Dir*8,Pos-Dir*6-Side*5,White,3);Line(Pos-Dir*6+Side*5,Pos-Dir*6-Side*5,White,2);
  }E.PopClip();Border(TL.X,TL.Y,Size.X,Size.Y,Gold);
 }
 void ButtonSkin(float SX,float SW,float X,float Y,float Width,float Height)const{Skin(SX,451,SW,86,40,24,.5f,X,Y,Width,Height);}
 void Button(FString Label,FString Id,float X,float Y,float Width=260,float Height=46,bool Enabled=true)const{
  bool Hover=FBox2D(FVector2D(X,Y),FVector2D(X+Width,Y+Height)).IsInside(W->Mouse);
  if(W->Host->Texture("ui_main"))ButtonSkin(Enabled?(Hover?247:20):700,Enabled?218:214,X,Y,Width,Height);else{Rect(X,Y,Width,Height,Ink);Border(X,Y,Width,Height);}
  TextBox(Label,X+21,Y+8,Width-42,Height-16,18,Enabled?White:Muted);if(Enabled)W->Hits.Add({FBox2D(FVector2D(X,Y),FVector2D(X+Width,Y+Height)),Id});
 }
 void Nav(FString Label,FString Id,int32 IconIndex,float X,float Y,float Width,float Height)const{Button("",Id,X,Y,Width,Height);ArtFit(23+IconIndex*128,735,121,116,X+5,Y+5,Height-10,Height-10);TextBox(Label,X+Height,Y+9,Width-Height-12,Height-18,16,White);}
 FBox2D Image(FString Id,float X,float Y,float Width,float Height,float Opacity=1,bool Stretch=false)const{auto T=W->Host->Texture(Id);if(!T)return FBox2D(FVector2D(X,Y),FVector2D(X+Width,Y+Height));float Ratio=FMath::Min(Width/T->GetSizeX(),Height/T->GetSizeY());bool Panorama=Id=="mainTownSquare"&&X==130;float IW=(Panorama||Stretch)?Width:T->GetSizeX()*Ratio,IH=(Panorama||Stretch)?Height:T->GetSizeY()*Ratio;float IX=X+(Width-IW)/2,IY=Y+(Height-IH)/2;FSlateBrush& B=W->ImageBrushes.FindOrAdd(Id);B.SetResourceObject(T);B.ImageSize=FVector2D(T->GetSizeX(),T->GetSizeY());B.DrawAs=ESlateBrushDrawType::Image;B.SetUVRegion(FBox2f(FVector2f(0,0),FVector2f(1,Panorama?Height*T->GetSizeX()/(Width*T->GetSizeY()):1)));FSlateDrawElement::MakeBox(E,L++,G.ToPaintGeometry(FVector2D(IW,IH),FSlateLayoutTransform(FVector2D(IX,IY))),&B,ESlateDrawEffect::None,FLinearColor(1,1,1,Opacity));return FBox2D(FVector2D(IX,IY),FVector2D(IX+IW,IY+IH));}
 void Bar(float X,float Y,float Width,float Fraction,FLinearColor C)const{Rect(X,Y,Width,7,FLinearColor(.08f,.07f,.07f,1));Rect(X,Y,Width*FMath::Clamp(Fraction,0.f,1.f),7,C);}
 void Vital(float X,float Y,float Width,float Fraction,bool Mana)const{Skin(691,644,325,58,55,20,20.f/58,X,Y,Width,20);float F=FMath::Clamp(Fraction,0.f,1.f);TileArt(Mana?407:70,668,220,12,X+13,Y+6,(Width-26)*F,8,8.f/12);}

 void Atlas(FString Id,int32 Col,int32 Row,int32 Cols,int32 Rows,float X,float Y,float Size)const{
  auto T=W->Host->Texture(Id);if(!T)return;FString Key=Id+FString::Printf(TEXT("_%d_%d"),Col,Row);auto& B=W->ImageBrushes.FindOrAdd(Key);B.SetResourceObject(T);B.ImageSize=FVector2D(Size,Size);B.DrawAs=ESlateBrushDrawType::Image;
  B.SetUVRegion(FBox2f(FVector2f((Col+.07f)/Cols,(Row+.07f)/Rows),FVector2f((Col+.93f)/Cols,(Row+.93f)/Rows)));
  FSlateDrawElement::MakeBox(E,L++,G.ToPaintGeometry(FVector2D(Size,Size),FSlateLayoutTransform(FVector2D(X,Y))),&B,ESlateDrawEffect::None,FLinearColor::White);
 }
 void Icon(const FBagItem& Item,float X,float Y,float Size)const{
  const TArray<FString> Weapons={"dagger","sword","broadsword","axe","mace","spear","bow","crossbow","holystaff","magestaff"};const TArray<FString> Armor={"cloth","leather","studded","chain","scale","scaleplate","plate","ornate","priestrobes","magerobes"};int32 Col=Weapons.Find(Item.Id);if(Col>=0){Atlas("weapon_icons",Col,Item.Quality,10,5,X,Y,Size);return;}Col=Armor.Find(Item.Id);if(Col>=0){Atlas("bodyarmor_icons",Col,Item.Quality,10,5,X,Y,Size);return;}
  auto D=W->Host->Model->Item(Item.Id);if(D&&!D->Slot.IsEmpty()){FString AtlasId=D->Slot=="Head"?"helmet_icons":D->Slot=="Hands"?"glovearmor_icons":D->Slot=="Feet"?"bootarmor_icons":D->Slot=="Off Hand"?"offhand_icons":"accessories_icons";Atlas(AtlasId,Item.Id=="charm"?11:0,Item.Quality,AtlasId=="accessories_icons"?12:10,5,X,Y,Size);return;}
  if(Item.Id=="health")Atlas("foods_potions_etc_icons",0,0,12,5,X,Y,Size);else if(Item.Id=="mana")Atlas("foods_potions_etc_icons",1,0,12,5,X,Y,Size);else if(Item.Id=="greater_health")Atlas("foods_potions_etc_icons",2,1,12,5,X,Y,Size);else if(Item.Id=="greater_mana")Atlas("foods_potions_etc_icons",3,1,12,5,X,Y,Size);else if(Item.Id=="food")Atlas("foods_potions_etc_icons",8,1,12,5,X,Y,Size);else if(Item.Id=="remedy")Atlas("foods_potions_etc_icons",2,0,12,5,X,Y,Size);else Text(Item.Id.Left(7),X+3,Y+Size/3,12);
 }
};
}
int32 UDungeonWidget::NativePaint(const FPaintArgs& Args,const FGeometry& Geometry,const FSlateRect& Clip,FSlateWindowElementList& Elements,int32 Layer,const FWidgetStyle& Style,bool Enabled)const{
 if(!Host||!Host->Model)return Layer;auto M=Host->Model.Get();auto S=M->State.Get();Hits.Empty();float Scale=FMath::Min(Geometry.GetLocalSize().X/1600.f,Geometry.GetLocalSize().Y/900.f);FVector2D Offset=(Geometry.GetLocalSize()-FVector2D(1600,900)*Scale)/2;FGeometry G=Geometry.MakeChild(FVector2D(1600,900),FSlateLayoutTransform(Scale,Offset));FPainter P{G,Elements,Layer,this};FString Screen=Host->WorldLoading?"Loading":M->Screen;
 bool Dungeon=Screen=="Dungeon";bool Town=Screen=="Town";bool Main=Screen=="Menu";const bool Inventory=Screen=="Inventory"||Screen=="DropConfirm";
 if(Screen=="Pause"||Main){
  // In the dungeon the live world remains visible; town and startup use the town panorama.
  if(Main||S->InTown)if(auto T=Host->Texture("mainTownSquare")){
   FPainter Background{Geometry,Elements,P.L,this};const FVector2D Size=Geometry.GetLocalSize();
   const float Cover=FMath::Max(Size.X/T->GetSizeX(),Size.Y/T->GetSizeY());
   const float Width=T->GetSizeX()*Cover,Height=T->GetSizeY()*Cover;
   Background.Image("mainTownSquare",(Size.X-Width)/2,(Size.Y-Height)/2,Width,Height);P.L=Background.L;
  }
  P.Image("main_menu_parchment",545,220,510,460,1.f,true);
  P.SmokeBorder(545,220,510,460,Host->AtmosphereClock);
  const FString Labels[]={Main?"New Game":"Resume",Main?"Continue":"Manual Save","Load Game","Settings",Main?"Credits":"Main Menu","Quit"};
  const FString Actions[]={Main?"New":"Resume",Main?"Continue":"Save","Load","Settings",Main?"Credits":"Menu","Quit"};
  for(int32 I=0;I<6;++I){
   const float X=625,Y=333+I*48,Width=350,Height=40;
   const bool Active=!(Main&&I==1)||M->Characters.ContainsByPredicate([](const FJourneySummary& J){return !J.Ended&&!J.Slot.IsEmpty();});
   const bool Hover=FBox2D(FVector2D(X,Y),FVector2D(X+Width,Y+Height)).IsInside(Mouse);
   P.ButtonSkin(Active?(Hover?247:20):700,Active?218:214,X,Y,Width,Height);
   P.CenterText(Labels[I],X,Y,Width,Height,15,Active?White:Muted);
   if(Active)Hits.Add({FBox2D(FVector2D(X,Y),FVector2D(X+Width,Y+Height)),Actions[I]});
  }
  // Draw the supplied transparent title last so it sits above every menu layer.
  P.Image(Main?"lonemoore_title_bar":"paused_title_bar",490,140,620,620.f/3.f);
  if(!M->SaveToast.IsEmpty()&&FPlatformTime::Seconds()<M->SaveToastUntil){P.Rect(565,42,470,62,FLinearColor(.025f,.030f,.026f,.97f));P.Frame(565,42,470,62);P.CenterText(M->SaveToast,585,52,430,42,21,Gold);}
 return P.L+1;
 }
 if(!Dungeon&&!Inventory){P.Rect(0,0,1600,900,Ink);FString Art=Town?"mainTownSquare":Screen=="Select"?"dungeonEntrance":Screen=="Service"?M->Service:Screen=="Choice"||Screen=="Ending"||Screen=="Stinger"?"dungeonEntrance":"mainTownSquare";P.Image(Art,Town?130:0,Town?78:0,Town?1340:1600,Town?585:900,Main?.55f:Screen=="Service"||Town?1.f:.24f);}
 if(Screen=="Loading"){
  // Preserve the complete composition and its baked-in title and loading label.
  FPainter Backdrop{Geometry,Elements,P.L,this};Backdrop.Rect(0,0,Geometry.GetLocalSize().X,Geometry.GetLocalSize().Y,FLinearColor::Black);P.L=Backdrop.L;
  P.Image("loadingscreen",0,0,1600,900);
  if(!Host->Texture("loadingscreen"))P.Text("Loading...",700,810,25);
  const float Progress=Host->WorldLoading?Host->WorldLoadProgress:FMath::Clamp(float(Host->LoadedAssets)/FMath::Max(1,Host->StartupAssets.Num()),0.f,1.f);
  P.Rect(620,873,360,3,Ink);P.Rect(620,873,360*Progress,3,Gold);
  if(!M->SaveToast.IsEmpty()&&FPlatformTime::Seconds()<M->SaveToastUntil){P.Rect(565,42,470,62,FLinearColor(.025f,.030f,.026f,.97f));P.Frame(565,42,470,62);P.CenterText(M->SaveToast,585,52,430,42,21,Gold);}
 return P.L+1;
 }
 if(Screen=="Select"){
  P.Text("Choose the one who heard the call",155,48,39,Gold);P.Text("All seven classes are available. Your class is permanent; companions join below.",160,105,19,Muted);
  for(int32 I=0;I<M->Classes.Num();++I){float X=158+(I%4)*325,Y=155+(I/4)*310;
   P.Rect(X,Y,298,290,Ink);P.Frame(X,Y,298,290);P.Image(M->Classes[I].Art,X+48,Y+12,202,158);
   P.TextBox(M->Classes[I].Name,X+19,Y+171,260,35,27,Gold);P.TextBox(M->Classes[I].Passive,X+19,Y+210,260,32,14,Muted);
   P.Button("Choose "+M->Classes[I].Name,"Hero:"+FString::FromInt(I),X+17,Y+245,264,38);
  }
  P.Button("Back","Back",1190,805,265);P.Text("Next: give your character a name and choose difficulty.",160,812,19,Muted);
 }else if(Screen=="NameHero"){
  const auto& Class=M->Classes[M->NewHeroClass];P.Text("NAME YOUR CHARACTER",170,82,40,Gold);P.Text("Their journey and saves will be kept together under this name.",175,146,21,Muted);
  P.Image(Class.Art,170,222,330,425);P.TextBox(Class.Name,185,674,310,50,32,Gold);P.Lines(Class.Passive,185,735,19,30,Muted);
  P.Text("Character name",595,251,27,Gold);P.Rect(590,310,785,76,Ink);P.Border(590,310,785,76,Gold);
  if(NameSelectAll&&!M->NewHeroName.IsEmpty())P.Rect(610,324,745,46,FLinearColor(.23f,.20f,.12f,.85f));
  P.Text(M->NewHeroName.IsEmpty()?TEXT("Type your name here"):M->NewHeroName,610,331,27,M->NewHeroName.IsEmpty()?Muted:White);
  if(HasKeyboardFocus()&&FMath::Fmod(FPlatformTime::Seconds(),1.1)<.65){auto Font=FCoreStyle::GetDefaultFontStyle("Regular",27);float CaretX=FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(M->NewHeroName.Left(FMath::Clamp(NameCaret,0,M->NewHeroName.Len())),Font).X;P.Rect(610+CaretX,328,2,38,Gold);}
  Hits.Add({FBox2D(FVector2D(590,310),FVector2D(1375,386)),"NameFocus"});
  FString Error;bool Valid=M->ValidateCharacterName(M->NewHeroName,Error);P.Text("24 characters maximum / Ctrl+A selects all / Ctrl+V pastes",595,405,17,Muted);
  if(!M->NameError.IsEmpty())Error=M->NameError;
  if(!Error.IsEmpty())P.Lines(Error,595,451,20,65,Gold);
  P.Button(M->HardcoreChoice?"Hardcore / permanent death":"Normal difficulty","Difficulty",590,556,520,55);
  P.Lines(M->HardcoreChoice?"A fallen hero stays dead. Your protagonist's death ends this journey.":"Return to Lonemoore after defeat and recover what you lost.",595,636,20,61,Muted);
  P.Button("Begin journey","BeginNamed",590,756,430,58,Valid);P.Button("Change class","NameBack",1045,756,330,58);
 }else if(Screen=="Intro"){
  const FString Panels[]={"You travelled alone. No contract, no promise of gold. Only the dream: a bell ringing beneath the earth.","Every road led to Lonemoore. They said the city had buried its dead so deeply that the dead had forgotten the sky.","Then the dream spoke your name. At the gates of Lonemoore, you finally understood: the darkness had pulled you here."};
  P.Image(M->IntroPanel==0?"dungeonEntrance":M->IntroPanel==1?"backAlley":"mainTownSquare",0,0,1600,680);P.Rect(0,640,1600,260,Ink);P.Text(FString::Printf(TEXT("THE CALL  /  %d OF 3"),M->IntroPanel+1),140,675,18,Gold);P.Lines(Panels[FMath::Clamp(M->IntroPanel,0,2)],140,720,25,85);P.Button(M->IntroPanel==2?"Enter Lonemoore":"Continue","IntroNext",1210,820,250);
 }else if(Town){
  P.Rect(125,5,1350,72,Ink);P.Frame(125,5,1350,72);P.TextBox("LONEMOORE",158,16,270,50,26,Gold);P.TextBox(S->Postgame?"The portal remains. Hell Hunts await.":"The last light above the descent.",440,16,740,50,17,Muted);
  struct Spot{const char* Label;const char* Id;float X,Y,W,H;};
  const Spot Spots[]={{"Blacksmith","blacksmith",.015f,.39f,.15f,.30f},{"Healer","healer",.37f,.13f,.17f,.26f},{"Dungeon Entrance","dungeonEntrance",.60f,.30f,.105f,.30f},{"Merchant","merchant",.23f,.35f,.15f,.26f},{"Tavern","tavern",.82f,.24f,.17f,.36f},{"Back Alley","backAlley",.169f,.34f,.045f,.23f},{"Hunt Board","huntBoard",.40f,.38f,.16f,.25f}};
  // The town image uses an aspect-preserving top-aligned crop; hotspots use its uncropped scale.
  auto Tex=Host->Texture("mainTownSquare");float IW=1340,IH=585,IX=130,IY=78;if(Tex){float R=1340.f/Tex->GetSizeX();IW=Tex->GetSizeX()*R;IH=Tex->GetSizeY()*R;IX=130;IY=78;}
  for(auto H:Spots){float X=IX+IW*H.X,Y=IY+IH*H.Y,W=IW*H.W,V=IH*H.H;FBox2D Box(FVector2D(X,Y),FVector2D(X+W,Y+V));Hits.Add({Box,"Service:"+FString(H.Id)});if(Box.IsInside(Mouse)){P.Rect(X,Y,W,V,FLinearColor(.8f,.61f,.29f,.15f));P.Border(X,Y,W,V,Gold);P.Button(H.Label,"Service:"+FString(H.Id),FMath::Clamp(X,20.f,1300.f),Y+V,270);}}
  P.Frame(125,72,1350,598);P.Frame(0,0,125,733);P.Frame(1476,0,124,733);
  P.ArtFit(25,857,213,229,20,15,85,92);P.ArtFit(248,857,227,229,1496,15,85,86);
  P.ArtFit(495,857,127,221,30,230,65,145);P.StoneText("People",25,420,75,22,12,Gold);P.StoneText("go deeper.",25,442,75,22,12,Gold);P.StoneText("Some",25,480,75,22,12,Gold);P.StoneText("never return.",25,502,75,22,11,Gold);P.StoneText("Faith",1499,240,75,22,12,Gold);P.StoneText("holds the",1499,262,75,22,12,Gold);P.StoneText("night.",1499,284,75,22,12,Gold);
  const FString Labels[]={"Blacksmith","Merchant","Healer","Tavern","Hunt Board","Back Alley","Dungeon"},Ids[]={"blacksmith","merchant","healer","tavern","huntBoard","backAlley","dungeonEntrance"};for(int32 I=0;I<7;++I)P.Nav(Labels[I],"Service:"+Ids[I],I,130+I*192,676,188,54);

 }else if(Screen=="Service"){
  // Keep x=424..1228 clear for the merchant, including their hands and held objects.
  struct FOffer{FString Name,Summary,Details,Action,Label,ExtraAction,ExtraLabel;FBagItem Item;int32 Price=-1;bool Enabled=true;};
  TArray<FOffer> Offers;FString Title=M->Service,Intro;
  auto Add=[&](FString Name,FString Summary,FString Details,FString Action,FString Label,int32 Price=-1,bool Active=true)->FOffer&{FOffer O;O.Name=Name;O.Summary=Summary;O.Details=Details;O.Action=Action;O.Label=Label;O.Price=Price;O.Enabled=Active&&(Price<0||S->Gold>=Price);Offers.Add(O);return Offers.Last();};
  auto ItemDetails=[&](const FBagItem& B){
   auto D=M->Item(B.Id);FString Details=Qualities[FMath::Clamp(B.Quality,0,4)]+"\n"+M->ItemDetails(B)+FString::Printf(TEXT("\nStack: %d / %d"),B.Count,D->Stack);
   if(!D->Slot.IsEmpty()&&S->Party.IsValidIndex(M->SelectedHero)){const auto& H=S->Party[M->SelectedHero];const auto& C=M->Classes[H.Class];bool Allowed=D->Slot=="Weapon"?C.Weapons.Contains(D->Family):D->Slot=="Body"?C.Armor.Contains(D->Family):D->Slot=="Off Hand"?(H.Class==0||H.Class==5):true;Details+="\n"+M->HeroName(H)+(Allowed?": class can equip":": class cannot equip");}
   return Details;
  };
  if(M->Service=="merchant"){
   Title="Merchant";Intro=Host->SellMode?"Sell from any party member. Equipped items are marked.":"Provisions and equipment for the road below.";
   if(Host->SellMode){for(int32 H=0;H<S->Party.Num();++H)if(!S->Hardcore||S->Party[H].HP>0){
    auto Sale=[&](int32 SaleSlot,const FBagItem& B){auto D=M->Item(B.Id);if(!D)return;FString Owner=M->HeroName(S->Party[H]);FString Details=ItemDetails(B)+"\nOwner: "+Owner+(SaleSlot>=100?"\nEQUIPPED: selling removes this item and reduces the hero's stats.":"\nSells one item from this stack.");auto& O=Add(D->Name,Owner+(SaleSlot>=100?" / EQUIPPED":FString::Printf(TEXT(" / x%d"),B.Count)),Details,FString::Printf(TEXT("Sell:%d:%d"),H,SaleSlot),"Sell one",M->SellPrice(B));O.Item=B;O.Enabled=true;};
    for(int32 J=0;J<S->Party[H].Bag.Num();++J)Sale(J,S->Party[H].Bag[J]);for(int32 J=0;J<S->Party[H].Gear.Num();++J)Sale(100+J,S->Party[H].Gear[J]);
   }}else for(const auto& D:M->Items){FBagItem B(D.Id);auto& O=Add(D.Name,M->ItemDetails(B),ItemDetails(B)+"\nPurchase: one item. Equipment is placed in a party bag; equip it from Inventory.","Buy:"+D.Id,"Buy one",D.Value);O.Item=B;}
  }else if(M->Service=="blacksmith"){
   Title="Blacksmith";Intro="Improve equipped gear. Choose a party member below.";
   if(S->Party.IsValidIndex(M->SelectedHero)){const auto& H=S->Party[M->SelectedHero];for(int32 I=0;I<H.Gear.Num();++I)if(auto D=M->Item(H.Gear[I].Id)){
    auto B=H.Gear[I],Next=B;Next.Quality=FMath::Min(4,B.Quality+1);auto Upgraded=H;Upgraded.Gear[I]=Next;
    FString Details=ItemDetails(B)+"\nEquipped by "+M->HeroName(H)+"\n\n"+(B.Quality==4?FString("Maximum quality reached."):"Upgrade to "+Qualities[Next.Quality]+FString::Printf(TEXT("\nItem power: %d > %d\nHero attack: %d > %d\nHero armor: %d > %d"),M->ItemPower(B),M->ItemPower(Next),M->Attack(H),M->Attack(Upgraded),M->Armor(H),M->Armor(Upgraded)));
    auto& O=Add(D->Name,Qualities[B.Quality]+" / "+D->Slot,Details,"Upgrade:"+FString::FromInt(I),B.Quality==4?"Maximum quality":"Upgrade",B.Quality==4?-1:D->Value*(B.Quality+1)*2,B.Quality<4);O.Item=B;
   }}
  }else if(M->Service=="tavern"||M->Service=="healer"){
   const bool Tavern=M->Service=="tavern";Title=Tavern?"The Tavern":"Healer";Intro=Tavern?"A fire, a bowl of stew, and a room with no whispers in the walls.":"There is still a little mercy in Lonemoore.";
   auto Care=[&](FString Name,FString Summary,FString Details,FString Action,FString Label){bool Needs=M->NeedsCare(Action);bool Allowed=Action!="resurrect"||!S->Hardcore;Add(Name,Summary,Details+(!Allowed?"\n\nDeath is permanent in Hardcore.":!Needs?"\n\nNo hero needs this care.":""),"Care:"+Action,Label,M->ServicePrice(Action),Needs&&Allowed);};
   Care("Heal living heroes","Health & cleansing","Restore full HP and clear status effects for all living heroes.\n\nDoes not restore MP or revive fallen heroes.","heal","Heal party");
   if(Tavern){Care("Rest for the night","Full health & mana","Restore full HP and MP and clear status effects for all living heroes.\n\nFallen heroes are not revived.","rest","Take a room");Care("Warm meal","+10% damage / 3 fights","Your party deals 10% more damage for the next three fights.\n\nAnother meal refreshes the duration; the bonus does not stack.","meal","Order a meal");Add("Travel supplies","Visit the merchant","Browse equipment and provisions. Each item has its own price and details.","Service:merchant","Browse supplies");}
   else Care("Resurrect fallen heroes","Restore fallen companions","Restore fallen heroes to full HP and MP.\n\nUnavailable in Hardcore.","resurrect","Resurrect party");
  }else if(M->Service=="gambler"){
   Title="The Gambler";Intro="Sealed equipment. No promises, traveller.";Add("Sealed equipment","A mystery purchase","Receive one random equipment item.\n\nQuality chances:\nStandard 70%\nFine 24%\nMasterwork 5%\nLegendary 1%\n\nThe revealed item goes into a party bag.","Care:gamble","Buy sealed item",M->ServicePrice("gamble"));
  }else if(M->Service=="backAlley"){
   Title="Back Alley";Intro="A shutter slides back. A quiet offer from the shadows.";Add("Visit the Gambler","Sealed equipment","A voice offers things their owners will never come looking for.\n\nInspect the gambler's offer before spending any gold.","Service:gambler","Visit the Gambler");
  }else if(M->Service=="dungeonEntrance"){
   Title="Dungeon Entrance";Intro="Last Dawn Cathedral stands above the buried city.";Add("Begin the descent","Enter / choose waypoint","Its bell has been silent for a century. The stairs beneath it are open.\n\nTeleport freely to town outside combat [T]. Return only to an activated shrine.","Enter","Enter the dungeon");
  }else if(M->Service=="huntBoard"){
   Title=S->Postgame?"Hell Hunts":"Hunt Board";Intro="Persistent offers. Up to five active hunts.";
   for(int32 I=0;I<S->Hunts.Num();++I){const auto& H=S->Hunts[I];auto& O=Add(H.Name,FString::Printf(TEXT("%d / %d   |   %d gold"),H.Progress,H.Goal,H.Gold),FString::Printf(TEXT("Progress: %d / %d\nReward: %d gold + %d XP\n\n%s"),H.Progress,H.Goal,H.Gold,H.XP,H.Active?TEXT("Accepted hunt. Return here when complete to claim the reward."):TEXT("Accept this hunt to track progress.")),(H.Active?"Claim:":"Accept:")+FString::FromInt(I),H.Active?(H.Progress>=H.Goal?"Claim reward":"In progress"):"Accept hunt",-1,!H.Active||H.Progress>=H.Goal);if(!H.Active){O.ExtraAction="Decline:"+FString::FromInt(I);O.ExtraLabel="Decline hunt";}}
  }
  const int32 Page=FMath::Clamp(Host->UIPage,0,FMath::Max(0,(Offers.Num()-1)/5)),Start=Page*5,Count=FMath::Min(5,Offers.Num()-Start);
  Host->UIPage=Page;
  FString Context=M->Service+FString::Printf(TEXT("/%d/%d/%d/%d"),Host->SellMode,Page,M->SelectedHero,Offers.Num());if(ShopContext!=Context){ShopContext=Context;ShopSelection=0;}ShopSelection=FMath::Clamp(ShopSelection,0,FMath::Max(0,Count-1));
  P.Rect(34,124,390,598,Ink);P.Frame(34,124,390,598);P.Text("LONEMOORE",58,148,12,Gold);P.TextBox(Title,58,173,342,47,32,Gold);P.Wrap(Intro,58,229,342,17,Muted);
  if(M->Service=="merchant")P.Button(Host->SellMode?"Browse shop":"Sell party items","SellMode",58,290,342,38);
  int32 Inspect=ShopSelection;
  for(int32 I=0;I<Count;++I){const auto& O=Offers[Start+I];float Y=342+I*62;FBox2D Bounds(FVector2D(54,Y),FVector2D(404,Y+57));bool Hover=Bounds.IsInside(Mouse);if(Hover)Inspect=I;P.Rect(54,Y,350,57,Hover||I==ShopSelection?FLinearColor(.15f,.105f,.055f,.96f):FLinearColor(.035f,.03f,.022f,.96f));P.Border(54,Y,350,57,Hover||I==ShopSelection?Gold:FLinearColor(.23f,.18f,.11f));float TextX=66;if(!O.Item.Id.IsEmpty()){P.Icon(O.Item,60,Y+7,42);TextX=110;}P.TextBox(O.Name,TextX,Y+5,306-TextX,25,18);P.TextBox(O.Price<0?"":FString::Printf(TEXT("%d g"),O.Price),319,Y+7,76,21,16,Gold);P.TextBox(O.Summary,TextX,Y+31,392-TextX,19,12,Muted);Hits.Add({Bounds,"ShopSelect:"+FString::FromInt(I)});}
  if(Offers.IsEmpty())P.Wrap("No items available. Select another party member or browse the shop.",58,359,342,18,Muted);
  if(Page>0)P.Button("Previous","Page:-1",54,661,123,37);P.TextBox(FString::Printf(TEXT("%d / %d"),Page+1,FMath::Max(1,(Offers.Num()+4)/5)),188,667,80,24,14,Muted);if(Start+Count<Offers.Num())P.Button("Next","Page:1",281,661,123,37);
  P.Rect(1228,218,342,504,Ink);P.Frame(1228,218,342,504);P.Text("AT YOUR SERVICE",1252,244,12,Gold);
  if(Offers.IsValidIndex(Start+Inspect)){
   const auto& O=Offers[Start+Inspect];const FString DetailKey=Context+O.Action;if(ShopDetailKey!=DetailKey){ShopDetailKey=DetailKey;ShopDetailOffset=0;}
   const float Top=P.Wrap(O.Name,1252,276,294,24,Gold)+14,Height=596-Top;
   Elements.PushClip(FSlateClippingZone(G.ToPaintGeometry(FVector2D(294,Height),FSlateLayoutTransform(FVector2D(1252,Top)))));
   const float Bottom=P.Wrap(O.Details,1252,Top-ShopDetailOffset,294,16,White);Elements.PopClip();ShopDetailMax=FMath::Max(0.f,Bottom+ShopDetailOffset-596);ShopDetailOffset=FMath::Min(ShopDetailOffset,ShopDetailMax);
   if(ShopDetailMax>0)P.TextBox("Scroll for full details",1252,596,294,18,11,Muted);
   if(!O.Enabled&&O.Price>S->Gold)P.TextBox("Not enough gold",1252,622,294,24,16,Gold);
   if(!O.ExtraAction.IsEmpty())P.Button(O.ExtraLabel,O.ExtraAction,1252,608,294,37);
   P.Button(O.Label+(O.Price>=0?FString::Printf(TEXT(" / %d gold"),O.Price):FString()),O.Action,1252,658,294,43,O.Enabled);
  }
  else P.Wrap("Select an offering to view its full details.",1252,288,294,18,Muted);
  P.Button("Town Square","Back",1300,30,270,46);
 }else if(Inventory){
  // The bag is a sheet laid over the current world, with the same party cards below it.
  const FLinearColor PaperInk(.012f,.007f,.004f,1.f),PaperMuted(.035f,.018f,.008f,1.f);
  P.Paper(30,30,1540,730);
  const bool CloseHover=FBox2D(FVector2D(1498,48),FVector2D(1546,96)).IsInside(Mouse);
  P.Line(FVector2D(1511,61),FVector2D(1533,83),CloseHover?Red:PaperInk,3);
  P.Line(FVector2D(1533,61),FVector2D(1511,83),CloseHover?Red:PaperInk,3);
  Hits.Add({FBox2D(FVector2D(1498,48),FVector2D(1546,96)),"Back"});
  if(!S->PendingLoot.IsEmpty())P.Button(FString::Printf(TEXT("Collect %d guardian rewards"),S->PendingLoot.Num()),"CollectLoot",1040,51,425,39);
  const float Column=FMath::Min(300.f,1480.f/FMath::Max(1,S->Party.Num()));
  for(int32 I=0;I<S->Party.Num();++I){const auto& H=S->Party[I];const float X=60+I*Column,Cell=FMath::Min(62.f,(Column-20)/4.f);
   P.TextBox(M->HeroName(H),X,99,Column-20,32,23,PaperInk);
   P.Text(FString::Printf(TEXT("%d / 20 slots"),H.Bag.Num()),X,136,14,PaperMuted);
   for(int32 J=0;J<20;++J){const float BX=X+(J%4)*Cell,BY=170+(J/4)*62;
    P.Rect(BX,BY,Cell-5,57,FLinearColor(.055f,.025f,.008f,.18f));P.Border(BX,BY,Cell-5,57,I==M->SelectedHero&&J==M->SelectedItem?PaperInk:FLinearColor(.035f,.018f,.008f,.8f));
    if(H.Bag.IsValidIndex(J)){const auto& B=H.Bag[J];P.Icon(B,BX+3,BY+3,FMath::Min(Cell-11,51.f));P.Rect(BX+Cell-27,BY+37,21,18,Ink);P.Text(FString::FromInt(B.Count),BX+Cell-23,BY+37,12,White);Hits.Add({FBox2D(FVector2D(BX,BY),FVector2D(BX+Cell-5,BY+57)),FString::Printf(TEXT("Item:%d:%d"),I,J)});}
   }
  }
  P.Line(FVector2D(60,495),FVector2D(1540,495),FLinearColor(.035f,.018f,.008f,.75f),1);
  if(S->Party.IsValidIndex(M->SelectedHero)&&S->Party[M->SelectedHero].Bag.IsValidIndex(M->SelectedItem)){
   const auto& H=S->Party[M->SelectedHero];const auto& B=H.Bag[M->SelectedItem];const auto D=M->Item(B.Id);
   if(D){const bool Accessory=D->Slot=="Accessory 1"||D->Slot=="Accessory 2";
    P.Icon(B,65,520,100);P.TextBox(Qualities[B.Quality]+" "+D->Name,180,518,555,38,25,PaperInk);P.TextBox(M->ItemDetails(B),180,562,555,48,17,PaperMuted);
    if(Accessory){P.Button("Equip to accessory 1","EquipAccessory:1",65,641,250);P.Button("Equip to accessory 2","EquipAccessory:2",330,641,250);P.Button("Drop","Drop",595,641,125);}
    else{P.Button("Use","Use",65,641,125);P.Button("Equip","Equip",205,641,125);P.Button("Drop","Drop",345,641,125);}
    P.Text("Transfer to",785,520,17,PaperMuted);int32 TransferIndex=0;
    for(int32 I=0;I<S->Party.Num();++I)if(I!=M->SelectedHero){P.Button(M->HeroName(S->Party[I]),"Transfer:"+FString::FromInt(I),785+(TransferIndex%3)*245,558+(TransferIndex/3)*54,230,43);++TransferIndex;}
   }
  }else P.Text("Select an item to use, equip, transfer, or drop it.",65,550,21,PaperMuted);
  P.TextBox(FString::Printf(TEXT("%d gold"),S->Gold),1245,706,280,30,21,PaperInk);
  if(Screen=="DropConfirm"){Hits.Empty();P.Paper(430,280,740,260);P.Text("Discard the selected item?",480,330,30,PaperInk);P.Button("Discard","ConfirmDrop",480,425,290);P.Button("Keep it","Screen:Inventory",800,425,290);}
 }else if(Screen=="Character"){
  auto H=S->Party[M->SelectedHero];P.Image(M->Classes[H.Class].Art,50,130,250,350);P.Text(M->HeroName(H)+" / "+M->Classes[H.Class].Name,350,130,36,Gold);P.Text(FString::Printf(TEXT("Level %d / XP %d of %d / %d points available"),H.Level,H.XP,M->NextXP(H.Level),H.Points),350,195,21);const FString Attr[]={"Strength","Dexterity","Vitality","Intelligence","Wisdom","Luck"};for(int32 I=0;I<6;++I){float Y=255+I*58;P.Text(Attr[I],355,Y,24);P.Text(FString::FromInt(H.Stats[I]),590,Y,24,Gold);P.Button("+","Allocate:"+FString::FromInt(I),650,Y-4,55,42,H.Points>0&&H.HP>0);}
  P.Text(FString::Printf(TEXT("HP %d / %d   MP %d / %d   Attack %d   Armor %d"),H.HP,M->MaxHP(H),H.MP,M->MaxMP(H),M->Attack(H),M->Armor(H)),50,595,19);
  P.Lines(M->Classes[H.Class].Passive,50,646,18,62,Muted);for(int32 I=0;I<3;++I){auto D=M->Skill(M->Classes[H.Class].Skills[I]);P.Text(D->Name+FString::Printf(TEXT("  /  %d MP  /  level %d"),D->Cost,D->Unlock),50,710+I*36,20,H.Level>=D->Unlock?Gold:Muted);}
  P.Text("EQUIPMENT",920,135,29,Gold);const FString Slots[]={"Weapon","Off Hand","Head","Body","Hands","Feet","Accessory 1","Accessory 2"};for(int32 I=0;I<8;++I){P.Border(850,198+I*64,55,55,Gold);if(!H.Gear[I].Id.IsEmpty())P.Icon(H.Gear[I],852,200+I*64,51);P.Text(Slots[I],920,205+I*64,18,Muted);auto D=M->Item(H.Gear[I].Id);P.Text(D?Qualities[H.Gear[I].Quality]+" "+D->Name:"Empty",1095,200+I*64,18);if(D)P.Text(M->ItemDetails(H.Gear[I]),1095,227+I*64,15,Muted);}
 }else if(Screen=="Map"){
  int Floor=Host->MapFloor>=0?Host->MapFloor:S->Floor;const auto& D=M->Floors[Floor];int W=D.Rows[0].Len();
  P.TextBox(D.Name,55,40,1150,50,30,Gold);P.Text("NORTH UP / EXPLORATION",55,98,16,Muted);
  FVector2D Focus=Floor==S->Floor?FVector2D(S->X+.5f,S->Y+.5f):FVector2D(W/2.f,D.Rows.Num()/2.f);
  P.Navigation(Floor,FVector2D(625,455),FVector2D(1110,630),Host->MapZoom,Focus-Host->MapPan/Host->MapZoom,false);
  P.Button("Previous floor","MapFloor:-1",1220,165,300);P.Button("Next floor","MapFloor:1",1220,220,300);P.Button("Recenter","MapCenter",1220,290,145);P.Button("Fit floor","MapFit",1375,290,145);
  P.Button("Zoom +","MapZoom:1",1220,350,145);P.Button("Zoom -","MapZoom:-1",1375,350,145);
  P.Text("Bright: visible",1220,430,20,Gold);P.Text("Dim: remembered",1220,465,20,Muted);P.Text("Black: unexplored",1220,500,20,Muted);
  P.Text("+ Shrine   > Stairs",1220,548,18);P.Text("Gold line: door / gate",1220,580,18,Gold);P.Text("X Detected enemy",1220,612,18,Red);P.Text("Diamond: your marker",1220,644,18,Blue);
  P.Text("Drag with middle mouse to pan. Scroll to zoom. Click explored ground to mark it. M closes.",80,803,19,Muted);
  P.Button("Radar -","RadarRange:-1",1220,710,145);P.Button("Radar +","RadarRange:1",1375,710,145);P.Text(FString::Printf(TEXT("Radar range: %d tiles"),M->RadarRange),1220,767,18);
 }else if(Screen=="Waypoints"){
  P.Text("ACTIVATED SHRINES",90,55,38,Gold);P.Text("Return to a known waypoint. Shrines do not restore health or mana.",90,115,21,Muted);TArray<int32> Points;for(int32 F=0;F<S->Floors.Num();++F)for(int32 C:S->Floors[F].Shrines)Points.Add(F*10000+C);int32 Start=Host->UIPage*9;for(int32 I=Start;I<FMath::Min(Start+9,Points.Num());++I){int32 F=Points[I]/10000,C=Points[I]%10000;P.Button(M->Floors[F].Name+(M->Floors[F].Rows[C/M->Floors[F].Rows[0].Len()][C%M->Floors[F].Rows[0].Len()]=='S'?" / entrance":" / descent"),"Waypoint:"+FString::FromInt(Points[I]),90,195+(I-Start)*63,1390,50);}if(Start+9<Points.Num())P.Button("Next","Page:1",1210,785,270);if(Start>0)P.Button("Previous","Page:-1",90,785,270);
 }else if(Screen=="Journal"){
  P.Text("HUNT JOURNAL",85,65,38,Gold);float Y=180;for(int32 I=0;I<S->Hunts.Num();++I){auto H=S->Hunts[I];if(!H.Active)continue;P.Text(H.Name,90,Y,28);if(H.Kind>=3)P.Text("Quarry location: "+M->Floors[H.TargetFloor].Name,90,Y+78,16,Gold);P.Text(FString::Printf(TEXT("Progress %d / %d  /  Reward %d gold and %d XP"),H.Progress,H.Goal,H.Gold,H.XP),90,Y+45,21,Muted);Y+=117;}if(Y==180)P.Text("No active hunts. Visit the Hunt Board in Lonemoore.",90,Y,24);P.Lines("Keys: "+FString::Join(S->Keys,TEXT(", ")),90,770,20,105,Gold);
 }else if(Screen=="Choice"){
  P.Image("final_boss",930,50,560,710);P.Text("ASTRA",90,130,66,Gold);P.Text("Cosmic Chaos Nephilim Queen",95,218,24);P.Lines("You heard my call. Every wall, every grave, every prayer above us was a lock. Stand beside me. Let us open them.",95,306,29,44);P.Button("Reject Astra. End the curse.","Choice:0",95,565,725,66);P.Button("Accept her power. Betray your companions.","Choice:1",95,655,725,66);P.Text("Your journey is saved in BeforeAstra.",95,761,19,Muted);
 }else if(Screen=="Ending"||Screen=="Stinger"){
  P.Text(Screen=="Stinger"?"THE PORTAL REMEMBERS":S->Ending==1?"THE FIRST DAWN":"THE NEW HELL LORD",125,250,52,Gold);P.Lines(Screen=="Stinger"?"The Queen is dead, but Hell is older than its rulers. The Hell Key warms in your hand. In Lonemoore, the old notices burn away. New hunts await.":S->Ending==1?"Astra falls. Her broken halo fades above the empty throne. In Lonemoore, the bells ring for the living. For a moment, the darkness lets go.":"Your companions fall. Lonemoore burns above a gate that will never close. Astra's power passes into you. The darkness that called your name now answers it.",130,365,28,75);P.Button(Screen=="Stinger"?"Return to Lonemoore / Hell Hunts":"Credits",Screen=="Stinger"?"Postgame":"EndNext",130,620,650,60);
 }else if(Screen=="Credits"){
  P.CenterText("Lonemoore",230,190,1140,70,48,Gold);P.Text("Design and art direction / Project creator",460,330,27);P.Text("Existing artwork / supplied project collection",460,385,24);P.Text("Development / Codex",460,437,24);P.Text("Unreal Engine / Epic Games",460,489,24);P.Text("Modular geometry / Blender",460,541,24);P.Text("Original development audio / synthesized for this build",460,593,21,Muted);P.Button("Continue","CreditsNext",460,700,570);
 }else if(Screen=="Settings"){
  P.Rect(100,90,1400,730,FLinearColor(.22f,.105f,.04f,1));P.Rect(120,110,670,690,FLinearColor(.72f,.59f,.38f,1));P.Rect(805,110,675,690,FLinearColor(.76f,.64f,.43f,1));P.Border(115,105,1370,700,Gold);P.Rect(785,115,22,680,FLinearColor(.3f,.19f,.08f,1));
  P.Text("TRAVELLER'S HANDBOOK",165,140,32,Ink);P.Button("Graphics","SettingsPage:0",170,215,180);P.Button("Audio","SettingsPage:1",365,215,170);P.Button("Key bindings","SettingsPage:2",550,215,205);
  auto Settings=UGameUserSettings::GetGameUserSettings();
  if(Host->SettingsPage==0){P.Text("Graphics & display",170,300,28,Ink);P.Button("Toggle fullscreen / windowed","Fullscreen",170,355,560);P.Text("Quality",170,423,21,Ink);const FString Labels[]={"Low","Medium","High","Epic"};for(int32 I=0;I<4;++I)P.Button(Labels[I],"Quality:"+FString::FromInt(I),170+I*142,465,132,44);P.Text(FString("Quality: ")+(Settings->GetOverallScalabilityLevel()<0?FString("Custom"):Labels[FMath::Clamp(Settings->GetOverallScalabilityLevel(),0,3)])+FString::Printf(TEXT(" / Brightness: %.1f"),Host->Brightness),170,545,20,Ink);P.Button("Darker","Brightness:-1",170,590,265);P.Button("Brighter","Brightness:1",450,590,265);P.Button("Advanced graphics","SettingsPage:3",170,660,545);P.Text(FString::Printf(TEXT("%d x %d / %.0f FPS cap"),Settings->GetScreenResolution().X,Settings->GetScreenResolution().Y,Settings->GetFrameRateLimit()),865,265,18,Ink);P.Text("Resolution",865,300,25,Ink);P.Button("1280 x 720","Resolution:1280:720",865,355,510);P.Button("1600 x 900","Resolution:1600:900",865,415,510);P.Button("1920 x 1080","Resolution:1920:1080",865,475,510);P.Button(Settings->IsVSyncEnabled()?"VSync: on":"VSync: off","VSync",865,545,510);P.Button("30 FPS","FPS:30",865,610,155);P.Button("60 FPS","FPS:60",1035,610,155);P.Button("120 FPS","FPS:120",1205,610,170);}
  else if(Host->SettingsPage==1){P.Text("Audio levels",170,310,30,Ink);const FString Names[]={"Master","Music","Effects"};const float Values[]={Host->MasterVolume,Host->MusicVolume,Host->EffectsVolume};for(int32 I=0;I<3;++I){float Y=380+I*90;P.Text(Names[I]+FString::Printf(TEXT("  %d%%"),FMath::RoundToInt(Values[I]*100)),170,Y+8,25,Ink);P.Button("-","Volume:"+Names[I]+":-1",465,Y,80);P.Button("+","Volume:"+Names[I]+":1",570,Y,80);}P.Lines("Changes are saved automatically. Set a channel to zero to mute it. Effects include interface, footsteps and combat sounds.",865,355,25,33,Ink);P.Button("Test sound","TestSound",865,590,400);}
  else if(Host->SettingsPage==2){const FString Actions[]={"Forward","Backward","Left","Right","Interact","Town","Map","Inventory","Character","Journal","QuickSave","QuickLoad"};for(int32 I=0;I<12;++I){float X=I<6?170:865,Y=305+(I%6)*61;P.Text(Actions[I],X,Y+8,22,Ink);P.Button(Host->Bindings[Actions[I]].ToString(),"Bind:"+Actions[I],X+275,Y,230,44);}P.Button("Restore default keys","ResetBindings",865,695,505);P.Text("Escape cancels key capture. Duplicate keys swap.",170,695,17,Ink);if(!Host->BindingCapture.IsEmpty())P.Text("Press a key for "+Host->BindingCapture,865,755,23,Ink);}
  else {const FString Names[]={"Shadows","Textures","View distance","Anti-aliasing","Effects","Post processing"};const int32 Values[]={Settings->GetShadowQuality(),Settings->GetTextureQuality(),Settings->GetViewDistanceQuality(),Settings->GetAntiAliasingQuality(),Settings->GetVisualEffectQuality(),Settings->GetPostProcessingQuality()};for(int32 I=0;I<6;++I){float X=I<3?170:865,Y=330+(I%3)*110;P.Text(Names[I]+FString::Printf(TEXT(" / %d"),Values[I]),X,Y,23,Ink);P.Button("Lower","GraphicsOption:"+Names[I]+":-1",X,Y+38,240);P.Button("Higher","GraphicsOption:"+Names[I]+":1",X+260,Y+38,240);}P.Text("0 Low / 1 Medium / 2 High / 3 Epic",170,690,21,Ink);}
  P.Button("Close book","SettingsBack",170,738,300);if(Host->PageTurn>0){float Width=640*(Host->PageTurn/.3f);P.Rect(807,115,Width,615,FLinearColor(.85f,.74f,.52f,.7f));}

 }else if(Screen=="Load"||Screen=="SaveBrowser"){
  bool Saving=Screen=="SaveBrowser";P.Text(Saving?"SAVE JOURNEY":"LOAD JOURNEY",115,72,40,Gold);P.Text(Saving?"Create a new manual save. Previous manual saves are kept.":"Choose a character, then a saved moment from their journey.",120,132,20,Muted);
  P.Button("Back","BrowserBack",1230,67,240,45);
  if(Saving){P.TextBox(S->CharacterName,125,204,390,46,31,Gold);P.Button("Create new manual save","NewManual",120,282,395,57,!S->CharacterName.IsEmpty());P.Lines("Autosave and Quicksave each have their own slot for this character. Every manual save creates a separate entry.",130,374,21,31,Muted);}
  else{
   P.Text("CHARACTERS",125,202,21,Gold);
   if(M->Characters.IsEmpty())P.Lines("No characters yet. Start a new game to begin a journey.",130,300,22,29,Muted);
   for(int I=M->CharacterPage*6;I<FMath::Min(M->Characters.Num(),M->CharacterPage*6+6);++I){const auto& C=M->Characters[I];float Y=245+(I%6)*80;
    P.Button(C.Character,"Character:"+FString::FromInt(I),120,Y,395,43);if(M->BrowseCharacter==C.Character)P.Border(120,Y,395,43,Gold);
    P.TextBox(C.ClassName+FString::Printf(TEXT(" / Lv %d%s"),C.Level,C.Ended?TEXT(" / Journey ended"):C.Hardcore?TEXT(" / Hardcore"):TEXT("")),133,Y+44,365,26,15,Muted);
   }
   P.Button("Previous","CharacterPage:-1",120,754,185,43,M->CharacterPage>0);P.Button("Next","CharacterPage:1",330,754,185,43,(M->CharacterPage+1)*6<M->Characters.Num());
  }
  P.TextBox(M->BrowseCharacter.IsEmpty()?TEXT("SAVED MOMENTS"):M->BrowseCharacter+TEXT(" / SAVED MOMENTS"),585,202,875,34,22,Gold);
  if(M->SaveSlots.IsEmpty())P.Lines(M->BrowseCharacter.IsEmpty()?TEXT("Select a character to see their saves."):TEXT("No saves yet. Complete the introduction to reach the first autosave."),590,300,23,58,Muted);
  for(int I=M->SavePage*6;I<FMath::Min(M->SaveSlots.Num(),M->SavePage*6+6);++I){const auto& Row=M->SaveSlots[I];float Y=245+(I%6)*80;
   FDateTime Time;FString When=FDateTime::ParseIso8601(*Row.UpdatedUtc,Time)?Time.ToString(TEXT("%Y-%m-%d  %H:%M:%S UTC")):Row.UpdatedUtc;
   P.Button(Row.Label+TEXT("  /  ")+When,"SavedJourney:"+FString::FromInt(I),575,Y,890,43,!Row.Ended);
   P.TextBox(Row.ClassName+FString::Printf(TEXT(" / Lv %d / "),Row.Level)+Row.Location+(Row.Ended?TEXT(" / Journey ended"):TEXT("")),590,Y+44,860,26,16,Muted);
  }
  P.Button("Previous saves","SavePage:-1",575,754,280,43,M->SavePage>0);P.Button("Next saves","SavePage:1",1185,754,280,43,(M->SavePage+1)*6<M->SaveSlots.Num());
 }else if(Screen=="LoadConfirm"){
  P.TextBox("Load "+(M->PendingLoadCharacter.IsEmpty()?M->LoadSlot:M->LoadLabel)+"?",220,245,1160,90,39,Gold);P.Text("Unsaved progress will be replaced by this saved journey.",225,365,25);P.Button("Load journey","ConfirmLoad",225,480,470,60);P.Button("Cancel","CancelLoad",725,480,470,60);
 }else if(Screen=="GameOver"){
  P.Text("THE JOURNEY ENDS",180,240,52,Gold);P.Lines(M->Notice,185,350,26,76);P.Button("Load a journey","Load",185,535,540);P.Button("Main menu","Menu",755,535,430);
 }
 if(Dungeon){
  P.Navigation(S->Floor,FVector2D(1440,175),FVector2D(250,250),250.f/(M->RadarRange*2+1),FVector2D(S->X+.5f,S->Y+.5f),true);P.Frame(1308,43,264,264);P.Text("N",1433,23,17,Gold);

  if(M->Combat){
   int32 Count=M->Enemies.Num();float Spacing=FMath::Min(340.f,1430.f/FMath::Max(1,Count));float Start=800-(Count*Spacing)/2;
   for(int32 I=0;I<Count;++I){auto E=M->Enemies[I];if(E.HP<=0&&!M->CombatEffects.ContainsByPredicate([&](const UDungeonModel::FCombatEffect& Effect){return !Effect.Hero&&Effect.Target==I;}))continue;float X=Start+I*Spacing;P.Image(E.Art,X,155,Spacing,355);P.Button(E.Name.Left(26),"Target:"+FString::FromInt(I),X,526,Spacing-8,42);P.Bar(X,514,Spacing-8,float(E.HP)/E.MaxHP,I==M->Target?Gold:Red);if(I==M->Target)P.Text("TARGET",X+Spacing*.33f,126,15,Gold);}
   P.Text(FString::Printf(TEXT("ROUND %d"),M->Round),40,100,17,Muted);
   if(M->Acting<0){P.Text("ENEMY TURN / resolving actions",45,663,24,Gold);P.Button("1 Attack","Action:Attack",530,652,210,44,false);P.Button("2 Defend","Action:Defend",760,652,210,44,false);P.Button("Skills","Action:Skill:0",990,652,210,44,false);}
   if(S->Party.IsValidIndex(M->Acting)){auto H=S->Party[M->Acting];P.Text(M->HeroName(H)+" acts",45,602,22,Gold);P.Button("1 Attack","Action:Attack",255,596,145,44);P.Button("2 Defend","Action:Defend",414,596,145,44);P.Button("3 Health","Action:Item",573,596,174,44);P.Button("4 Mana","Action:Mana",761,596,170,44);P.Button("5 Flee","Action:Flee",945,596,125,44,!M->BossFight);for(int32 I=0;I<3;++I){auto Skill=M->Skill(M->Classes[H.Class].Skills[I]);P.Button(FString::Printf(TEXT("F%d "),I+1)+Skill->Name+FString::Printf(TEXT(" / %d MP"),Skill->Cost),"Action:Skill:"+FString::FromInt(I),255+I*415,653,400,43,H.Level>=Skill->Unlock&&H.MP>=Skill->Cost&&!H.Status.Contains("Silence"));}}
  }else{P.Rect(0,700,1600,34,FLinearColor(.025f,.029f,.035f,.8f));P.Text(M->Context(),35,707,18);}
 }
 FString StatusHover;float StatusHoverX=0;
 auto StatusBadges=[&](const auto& H,float X,float Y,float Width){
  TArray<FString> Keys;H.Status.GetKeys(Keys);Keys.Sort();int32 Badge=0;
  for(const FString& Key:Keys){const int32 Turns=H.Status.FindRef(Key);if(Turns<=0)continue;
   const int32 Columns=Keys.Num()>12?4:3;const float BadgeSize=Columns==4?14.f:18.f;const float Pitch=BadgeSize+1.f;const FVector2D At(X+5+(Badge%Columns)*Pitch,Y+66-(Badge/Columns)*Pitch);++Badge;
   DungeonStatusIcons::Draw(G,Elements,P.L,Key,At,BadgeSize);
   if(FBox2D(At,At+FVector2D(BadgeSize,BadgeSize)).IsInside(Mouse)){
    const FString Detail=Key=="Poison"?"Venom deals damage each turn.":Key=="Bleed"?"Wounds deal damage each turn.":Key=="Burn"?"Flames deal damage each turn.":Key=="Plague"?"Illness deals damage and reduces healing.":Key=="Slow"?"Acts later in the combat round.":Key=="Blind"?"Attacks are less accurate.":Key=="Curse"?"Deals less damage.":Key=="Weakness"?"Deals less damage.":Key=="Silence"?"Cannot use class skills.":Key=="Fear"?"Loses the next action.":Key=="Stun"?"Loses the next action.":Key=="Guard"?"Incoming damage is reduced.":Key=="Mark"?"Receives more damage.":"An active combat effect.";
    StatusHover=(Key=="Bleed"?FString("Bleeding"):Key)+FString::Printf(TEXT(" / %d turn%s\n"),Turns,Turns==1?TEXT(""):TEXT("s"))+Detail;StatusHoverX=FMath::Clamp(X,15.f,1080.f);
   }
  }
 };
 bool PartyBar=Town||Dungeon||Screen=="Service"||Inventory;
 if(PartyBar&&Screen!="Service"&&!S->Party.IsEmpty()){
  P.Rect(0,780,1600,120,Ink);P.Frame(2,779,1197,119);float CW=FMath::Min(S->Party.Num()==1?340.f:230.f,1160.f/S->Party.Num());
  for(int32 I=0;I<S->Party.Num();++I){auto H=S->Party[I];float X=15+I*(CW+5),Y=793;bool Active=M->Combat?M->Acting==I:M->SelectedHero==I;P.Rect(X,Y,CW,90,Ink);P.Image(M->Classes[H.Class].Art,X+5,Y+5,55,77,H.HP<=0?.3f:1.f);P.Frame(X,Y,65,90);P.TextBox(M->HeroName(H),X+71,Y+3,CW-84,23,18,Active?Gold:White);P.Text(FString::Printf(TEXT("Lv%d %s"),H.Level,*M->Classes[H.Class].Name),X+71,Y+27,12,Muted);P.Vital(X+69,Y+45,CW-77,float(H.HP)/M->MaxHP(H),false);P.Vital(X+69,Y+65,CW-77,float(H.MP)/M->MaxMP(H),true);P.Text(FString::FromInt(H.HP)+" HP",X+81,Y+49,8);P.Text(FString::FromInt(H.MP)+" MP",X+81,Y+69,8);if(Active)P.Border(X+2,Y+2,CW-4,86,Gold);StatusBadges(H,X,Y,CW);if(!M->Combat&&Screen!="DropConfirm")Hits.Add({FBox2D(FVector2D(X,Y),FVector2D(X+CW,Y+90)),"Party:"+FString::FromInt(I)});}
  if(!Inventory){P.Rect(1204,734,392,105,Ink);P.Frame(1204,734,392,105);P.TextBox("Action history",1227,753,345,20,13,Gold);Hits.Add({FBox2D(FVector2D(1204,734),FVector2D(1596,839)),"History"});int32 RecentStart=FMath::Max(0,M->Log.Num()-2);for(int32 I=RecentStart;I<M->Log.Num();++I){float Y=779+(I-RecentStart)*19;P.TextBox(P.Fit(M->Log[I],345,11),1227,Y,345,18,11,Muted);}}

 }
 if(Screen=="Service"&&!S->Party.IsEmpty()){
  P.Rect(30,802,1540,90,Ink);P.Frame(30,802,1540,90);
  P.TextBox(M->Notice,50,808,1148,19,13,Muted);
  const float CW=FMath::Min(230.f,1148.f/S->Party.Num());
  for(int32 I=0;I<S->Party.Num();++I){const auto& H=S->Party[I];const float X=50+I*CW;P.Image(M->Classes[H.Class].Art,X,834,34,48);P.TextBox(M->HeroName(H)+FString::Printf(TEXT(" / Lv%d"),H.Level),X+41,831,CW-50,21,15,I==M->SelectedHero?Gold:White);P.Bar(X+41,859,CW-55,float(H.HP)/M->MaxHP(H),Red);P.Bar(X+41,876,CW-55,float(H.MP)/M->MaxMP(H),Blue);P.TextBox(FString::Printf(TEXT("%d HP    %d MP"),H.HP,H.MP),X+41,852,CW-55,25,10);Hits.Add({FBox2D(FVector2D(X,830),FVector2D(X+CW-5,887)),"Party:"+FString::FromInt(I)});}
  P.Button("Action history","History",1228,808,318,31);P.Button("Inventory","Screen:Inventory",1228,849,103,36);P.Button("Character","Screen:Character",1335,849,109,36);P.Button("Hunts","Screen:Journal",1448,849,98,36);
 }
 bool Playing=!S->Party.IsEmpty()&&Screen!="Settings"&&Screen!="Menu"&&Screen!="Select"&&Screen!="NameHero"&&Screen!="Load"&&Screen!="SaveBrowser"&&Screen!="Intro";
 if(Playing&&!Inventory&&!M->Combat&&Screen!="Choice"&&Screen!="Ending"&&Screen!="Stinger"&&Screen!="Credits"){
  if(!Dungeon)P.TextBox(FString::Printf(TEXT("%d GOLD%s"),S->Gold,S->Hardcore?TEXT("  /  HARDCORE"):TEXT("")),1230,Town?16:Screen=="Service"?91:78,235,Town?50:36,18,Gold);
  if(!PartyBar){P.Button("Back","Back",1380,27,170,42);if(Screen=="Character")for(int32 I=0;I<S->Party.Num();++I)P.Button(M->HeroName(S->Party[I]),"Party:"+FString::FromInt(I),50+I*240,825,225,41);}
  else if(Screen!="Service"){P.Button("Inventory","Screen:Inventory",1208,847,127,43);P.Button("Character","Screen:Character",1337,847,134,43);P.Button("Hunts","Screen:Journal",1473,847,120,43);}
 }
 if(!M->Notice.IsEmpty()&&!Inventory&&Screen!="Service"&&Screen!="Intro"&&Screen!="Choice"&&Screen!="Ending"&&Screen!="Stinger"&&Screen!="GameOver"&&Screen!="Menu"){
  if(PartyBar){if(S->Party.Num()==1){P.Paper(365,789,827,102);P.Lines(M->Notice,388,810,17,76,Ink);}else{P.Paper(8,734,1185,43);P.Text(P.Fit(M->Notice,1135,14),30,746,14,Ink);}}else{P.Rect(0,869,1085,31,Ink);P.Text(M->Notice.Left(123),20,872,15,White);}
 }
  if(Screen=="Dungeon")for(const auto& Effect:M->CombatEffects){
   const float Duration=DungeonCombatVFX::Duration(Effect.Cue);FVector2D Center;float Size=280.f;
   if(Effect.Hero){float CW=FMath::Min(S->Party.Num()==1?340.f:230.f,1160.f/FMath::Max(1,S->Party.Num()));Center=FVector2D(15+Effect.Target*(CW+5)+CW*.5f,824);Size=150.f;}
   else {float Spacing=FMath::Min(340.f,1430.f/FMath::Max(1,M->Enemies.Num()));Center=FVector2D(800-M->Enemies.Num()*Spacing*.5f+(Effect.Target+.5f)*Spacing,330);Size=FMath::Min(340.f,Spacing*1.15f);}
   P.L=DungeonCombatVFX::Paint(Effect.Cue,P.G,P.E,P.L,Center,Size,FMath::Clamp(Effect.Age/Duration,0.f,1.f));
  }
 if(!StatusHover.IsEmpty()&&!Host->ShowHistory&&Host->PendingCommand.IsEmpty()&&Screen!="DropConfirm"){P.Paper(StatusHoverX,671,500,106);P.Wrap(StatusHover,StatusHoverX+20,688,460,16,Ink);}
 if(Host->ShowHistory){
  Hits.Empty();
  if(Screen=="Service"){
   P.Rect(1228,218,342,504,FLinearColor(.007f,.006f,.005f,1.f));P.Frame(1228,218,342,504);P.Text("JOURNEY HISTORY",1252,242,18,Gold);
   const int32 End=FMath::Max(0,M->Log.Num()-Host->HistoryPage*5),Start=FMath::Max(0,End-5);
   for(int32 I=Start;I<End;++I){const float Y=283+(I-Start)*66;Elements.PushClip(FSlateClippingZone(G.ToPaintGeometry(FVector2D(294,61),FSlateLayoutTransform(FVector2D(1252,Y)))));P.Wrap(M->Log[I],1252,Y,294,14,Muted);Elements.PopClip();}
   P.Button("Older","HistoryPage:1",1252,621,140,34,Start>0);P.Button("Newer","HistoryPage:-1",1400,621,146,34,Host->HistoryPage>0);P.Button("Close","History",1252,666,294,37);
  }else{P.Rect(180,160,1240,605,Ink);P.Frame(180,160,1240,605);P.Text("JOURNEY HISTORY",215,185,30,Gold);int32 End=M->Log.Num()-Host->HistoryPage*12;int32 Start=FMath::Max(0,End-12);for(int32 I=Start;I<End;++I)P.Text(P.Fit(M->Log[I],1150,17),215,250+(I-Start)*34,17);P.Button("Older","HistoryPage:1",215,705,200);P.Button("Newer","HistoryPage:-1",435,705,200);P.Button("Close","History",1170,705,200);}
 }
 if(!Host->PendingCommand.IsEmpty()){
  Hits.Empty();
  if(Screen=="Service"){P.Rect(1228,218,342,504,FLinearColor(.007f,.006f,.005f,1.f));P.Frame(1228,218,342,504);P.Text(Host->PendingCommand.StartsWith("Sell:")?"CONFIRM SALE":"CONFIRM PURCHASE",1252,246,17,Gold);P.Wrap(Host->ConfirmText,1252,299,294,19);P.Button("Confirm","Confirm",1252,607,294,43);P.Button("Cancel","Cancel",1252,662,294,43);}
  else{P.Rect(0,0,1600,900,FLinearColor(0,0,0,.65f));P.Rect(370,280,860,320,Ink);P.Frame(370,280,860,320);P.Lines(Host->ConfirmText,415,325,25,45,White);P.Button("Confirm","Confirm",415,510,350);P.Button("Cancel","Cancel",810,510,350);}
 }
 if(!M->SaveToast.IsEmpty()&&FPlatformTime::Seconds()<M->SaveToastUntil){const bool Shop=Screen=="Service";const float X=Shop?34:565,W=Shop?390:470;P.Rect(X,42,W,62,FLinearColor(.025f,.030f,.026f,.97f));P.Frame(X,42,W,62);P.TextBox(M->SaveToast,X+20,52,W-40,42,18,Gold);}
 return P.L+1;
}
void UDungeonWidget::InsertNameText(const FString& Text){
 if(!Host||Host->Model->Screen!="NameHero")return;auto M=Host->Model;
 if(NameSelectAll){M->NewHeroName.Empty();NameCaret=0;NameSelectAll=false;}
 NameCaret=FMath::Clamp(NameCaret,0,M->NewHeroName.Len());
 FString Insert;for(TCHAR C:Text)if(C>=32&&C!=127)Insert.AppendChar(C);
 Insert=Insert.Left(FMath::Max(0,24-M->NewHeroName.Len()));M->NewHeroName.InsertAt(NameCaret,Insert);NameCaret+=Insert.Len();M->NameError.Empty();
}
FReply UDungeonWidget::NativeOnKeyChar(const FGeometry& G,const FCharacterEvent& E){
 if(!Host||Host->Model->Screen!="NameHero")return Super::NativeOnKeyChar(G,E);
 if(!E.IsControlDown()&&!E.IsAltDown()&&E.GetCharacter()>=32)InsertNameText(FString::Chr(E.GetCharacter()));return FReply::Handled();
}
FReply UDungeonWidget::NativeOnKeyDown(const FGeometry& G,const FKeyEvent& E){
 if(!Host||Host->Model->Screen!="NameHero")return Super::NativeOnKeyDown(G,E);
 auto M=Host->Model;const FKey Key=E.GetKey();NameCaret=FMath::Clamp(NameCaret,0,M->NewHeroName.Len());
 if(Key==EKeys::Enter){Host->Command("BeginNamed");return FReply::Handled();}
 if(Key==EKeys::Escape){Host->Command("NameBack");return FReply::Handled();}
 if(E.IsControlDown()&&Key==EKeys::A){NameSelectAll=true;NameCaret=M->NewHeroName.Len();return FReply::Handled();}
 if(E.IsControlDown()&&Key==EKeys::V){FString Text;FPlatformApplicationMisc::ClipboardPaste(Text);InsertNameText(Text);return FReply::Handled();}
 if(E.IsControlDown()&&(Key==EKeys::C||Key==EKeys::X)){if(NameSelectAll){FPlatformApplicationMisc::ClipboardCopy(*M->NewHeroName);if(Key==EKeys::X){M->NewHeroName.Empty();NameCaret=0;NameSelectAll=false;M->NameError.Empty();}}return FReply::Handled();}
 if(Key==EKeys::BackSpace||Key==EKeys::Delete){
  if(NameSelectAll){M->NewHeroName.Empty();NameCaret=0;}else if(Key==EKeys::BackSpace&&NameCaret>0){M->NewHeroName.RemoveAt(--NameCaret,1);}else if(Key==EKeys::Delete&&NameCaret<M->NewHeroName.Len())M->NewHeroName.RemoveAt(NameCaret,1);
  NameSelectAll=false;M->NameError.Empty();return FReply::Handled();
 }
 if(Key==EKeys::Left){NameCaret=NameSelectAll?0:FMath::Max(0,NameCaret-1);NameSelectAll=false;}
 if(Key==EKeys::Right){NameCaret=NameSelectAll?M->NewHeroName.Len():FMath::Min(M->NewHeroName.Len(),NameCaret+1);NameSelectAll=false;}
 if(Key==EKeys::Home){NameCaret=0;NameSelectAll=false;}if(Key==EKeys::End){NameCaret=M->NewHeroName.Len();NameSelectAll=false;}
 return FReply::Handled();
}
FReply UDungeonWidget::NativeOnMouseMove(const FGeometry& G,const FPointerEvent& E){float Scale=FMath::Min(G.GetLocalSize().X/1600.f,G.GetLocalSize().Y/900.f);FVector2D Old=Mouse;Mouse=(G.AbsoluteToLocal(E.GetScreenSpacePosition())-(G.GetLocalSize()-FVector2D(1600,900)*Scale)/2)/Scale;if(MapDragging&&Host->Model->Screen=="Map"){Host->MapPan+=Mouse-Old;return FReply::Handled();}return FReply::Unhandled();}
FReply UDungeonWidget::NativeOnMouseButtonDown(const FGeometry& G,const FPointerEvent& E){NativeOnMouseMove(G,E);if(Host->Model->Screen=="Map"&&Mouse.X>=70&&Mouse.X<=1180&&Mouse.Y>=140&&Mouse.Y<=770){if(E.GetEffectingButton()==EKeys::MiddleMouseButton){MapDragging=true;return FReply::Handled().CaptureMouse(TakeWidget());}if(E.GetEffectingButton()==EKeys::LeftMouseButton){auto M=Host->Model;auto S=M->State;int F=Host->MapFloor>=0?Host->MapFloor:S->Floor;int W=M->Floors[F].Rows[0].Len();FVector2D Focus=F==S->Floor?FVector2D(S->X+.5f,S->Y+.5f):FVector2D(W/2.f,M->Floors[F].Rows.Num()/2.f);FDungeonMapTransform T{FVector2D(625,455)-Focus*Host->MapZoom+Host->MapPan,Host->MapZoom,W};int C=T.CellAt(Mouse);auto& R=S->Floors[F];if(R.MemoryTiles.Contains(C)&&R.MemoryTiles[C]!="#"){if(R.Markers.Contains(C))R.Markers.Remove(C);else R.Markers.Add(C);}return FReply::Handled();}}
 if(E.GetEffectingButton()==EKeys::LeftMouseButton)for(int I=Hits.Num()-1;I>=0;--I)if(Hits[I].Bounds.IsInside(Mouse)){Host->Command(Hits[I].Id);return FReply::Handled();}return Host->Model->Screen=="Map"?FReply::Handled():FReply::Unhandled();}
FReply UDungeonWidget::NativeOnMouseButtonUp(const FGeometry& G,const FPointerEvent& E){if(MapDragging&&E.GetEffectingButton()==EKeys::MiddleMouseButton){MapDragging=false;return FReply::Handled().ReleaseMouseCapture();}return Super::NativeOnMouseButtonUp(G,E);}
FReply UDungeonWidget::NativeOnMouseWheel(const FGeometry& G,const FPointerEvent& E){NativeOnMouseMove(G,E);if(Host->Model->Screen=="Service"&&Mouse.X>=1228&&Mouse.X<=1570&&Mouse.Y>=218&&Mouse.Y<=722&&!Host->ShowHistory&&Host->PendingCommand.IsEmpty()){ShopDetailOffset=FMath::Clamp(ShopDetailOffset-E.GetWheelDelta()*48.f,0.f,ShopDetailMax);return FReply::Handled();}if(Host->Model->Screen=="Map"){Host->Command(E.GetWheelDelta()>0?"MapZoom:1":"MapZoom:-1");return FReply::Handled();}return Super::NativeOnMouseWheel(G,E);}

FReply UDungeonWidget::NativeOnMouseButtonDoubleClick(const FGeometry& G,const FPointerEvent& E){NativeOnMouseMove(G,E);if(E.GetEffectingButton()==EKeys::LeftMouseButton&&Host->Model->Screen=="Inventory"&&!Host->ShowHistory&&Host->PendingCommand.IsEmpty())for(int32 I=Hits.Num()-1;I>=0;--I)if(Hits[I].Bounds.IsInside(Mouse)&&Hits[I].Id.StartsWith("Item:")){Host->Command(Hits[I].Id);Host->Command("Use");return FReply::Handled();}return NativeOnMouseButtonDown(G,E);}
