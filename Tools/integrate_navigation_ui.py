from pathlib import Path
r=Path(__file__).resolve().parents[1]/'Source/DungeonCrawler'
p=r/'DungeonUI.cpp';s=p.read_text();s=s.replace('#include "DungeonGame.h"','#include "DungeonGame.h"\n#include "DungeonNavigation.h"')
at=s.index(' void ButtonSkin(')
s=s[:at]+''' void Line(FVector2D A,FVector2D B,FLinearColor Color,float Thickness=2)const{TArray<FVector2D> Points{A,B};FSlateDrawElement::MakeLines(E,L++,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,Color,true,Thickness);}
 void Navigation(int Floor,FVector2D Center,FVector2D Size,float Zoom,FVector2D Focus,bool Radar)const{
  auto M=W->Host->Model.Get();auto S=M->State;const auto& R=S->Floors[Floor];int Width=M->Floors[Floor].Rows[0].Len();FDungeonMapTransform T{Center-Focus*Zoom,Zoom,Width};FVector2D TL=Center-Size/2;
  Rect(TL.X,TL.Y,Size.X,Size.Y,FLinearColor(.016,.020,.018,.98));E.PushClip(FSlateClippingZone(G.ToPaintGeometry(Size,FSlateLayoutTransform(TL))));
  for(const auto& Pair:R.MemoryTiles){int C=Pair.Key;FVector2D Pos=T.CellCenter(C);bool Live=Floor==S->Floor&&M->Visible.Contains(C);FString Tile=Pair.Value;if(Tile=="#")continue;
   Rect(Pos.X-Zoom/2+1,Pos.Y-Zoom/2+1,Zoom-2,Zoom-2,Live?FLinearColor(.30,.31,.25):FLinearColor(.11,.12,.10));
   FString Icon=Tile==">"||Tile=="A"?">":Tile=="S"||Tile=="V"?"+":Tile=="L"?"L":Tile=="K"?"k":Tile=="C"||Tile=="$"?"o":Tile=="!"?"i":"";
   if(!Icon.IsEmpty())TextBox(Icon,Pos.X-Zoom*.18f,Pos.Y-Zoom*.45f,Zoom*.8f,Zoom*.9f,FMath::Clamp(int(Zoom*.6f),8,24),Live?Gold:Muted);
   if(R.Markers.Contains(C)){Line(Pos+FVector2D(-5,0),Pos+FVector2D(0,-6),Blue);Line(Pos+FVector2D(0,-6),Pos+FVector2D(5,0),Blue);Line(Pos+FVector2D(5,0),Pos+FVector2D(0,6),Blue);Line(Pos+FVector2D(0,6),Pos+FVector2D(-5,0),Blue);}
  }
  for(const auto& Edge:R.Boundaries){const FString* Known=R.MemoryEdges.Find(Edge.Id);if(!Known||*Known=="Open"||Edge.B<0)continue;FVector2D A=T.CellCenter(Edge.A),B=T.CellCenter(Edge.B),Mid=(A+B)/2;FVector2D Span=FMath::Abs(A.X-B.X)>1?FVector2D(0,Zoom/2):FVector2D(Zoom/2,0);Line(Mid-Span,Mid+Span,*Known=="Wall"?Muted:Gold,*Known=="Wall"?2:4);if(*Known=="Bars")Line(Mid-Span*.5f+FVector2D(2,2),Mid+Span*.5f+FVector2D(2,2),White,1);}
  if(Floor==S->Floor){for(int C:M->DetectedEnemies){FVector2D Pos=T.CellCenter(C);Line(Pos+FVector2D(-4,-4),Pos+FVector2D(4,4),Red,3);Line(Pos+FVector2D(4,-4),Pos+FVector2D(-4,4),Red,3);}
   FVector2D Pos=T.CellCenter(M->Cell(S->X,S->Y));const FVector2D Directions[]={FVector2D(0,-1),FVector2D(1,0),FVector2D(0,1),FVector2D(-1,0)};FVector2D Dir=Directions[S->Facing],Side(-Dir.Y,Dir.X);Line(Pos+Dir*8,Pos-Dir*6+Side*5,White,3);Line(Pos+Dir*8,Pos-Dir*6-Side*5,White,3);Line(Pos-Dir*6+Side*5,Pos-Dir*6-Side*5,White,2);
  }E.PopClip();Border(TL.X,TL.Y,Size.X,Size.Y,Gold);
 }
'''+s[at:]
a=s.index('  P.Text(M->Floors[S->Floor].Name',s.index('}else if(Screen=="Map")'));b=s.index(' }else if(Screen=="Waypoints")',a)
s=s[:a]+'''  int Floor=Host->MapFloor>=0?Host->MapFloor:S->Floor;const auto& D=M->Floors[Floor];int W=D.Rows[0].Len();
  P.TextBox(D.Name,55,40,1150,50,30,Gold);P.Text("NORTH UP / EXPLORATION",55,98,16,Muted);
  FVector2D Focus=Floor==S->Floor?FVector2D(S->X+.5f,S->Y+.5f):FVector2D(W/2.f,D.Rows.Num()/2.f);
  P.Navigation(Floor,FVector2D(625,455),FVector2D(1110,630),Host->MapZoom,Focus-Host->MapPan/Host->MapZoom,false);
  P.Button("Previous floor","MapFloor:-1",1220,165,300);P.Button("Next floor","MapFloor:1",1220,220,300);P.Button("Recenter","MapCenter",1220,290,300);
  P.Button("Zoom +","MapZoom:1",1220,350,145);P.Button("Zoom -","MapZoom:-1",1375,350,145);
  P.Text("Bright: visible",1220,430,20,Gold);P.Text("Dim: remembered",1220,465,20,Muted);P.Text("Black: unexplored",1220,500,20,Muted);
  P.Text("+ Shrine   > Stairs",1220,548,18);P.Text("Gold line: door / gate",1220,580,18,Gold);P.Text("X Detected enemy",1220,612,18,Red);P.Text("Diamond: your marker",1220,644,18,Blue);
  P.Text("Drag with middle mouse to pan. Scroll to zoom. Click explored ground to mark it. M closes.",80,803,19,Muted);
  P.Button("Radar -","RadarRange:-1",1220,710,145);P.Button("Radar +","RadarRange:1",1375,710,145);P.Text(FString::Printf(TEXT("Radar range: %d tiles"),M->RadarRange),1220,767,18);
'''+s[b:]
s=s.replace(' if(Dungeon){\n',' if(Dungeon){\n  P.Navigation(S->Floor,FVector2D(1440,175),FVector2D(250,250),250.f/(M->RadarRange*2+1),FVector2D(S->X+.5f,S->Y+.5f),true);P.Text("N",1433,23,17,Gold);\n',1)
a=s.index('FReply UDungeonWidget::NativeOnMouseMove');b=s.index('\nFReply UDungeonWidget::NativeOnMouseButtonDoubleClick',a)
s=s[:a]+'''FReply UDungeonWidget::NativeOnMouseMove(const FGeometry& G,const FPointerEvent& E){float Scale=FMath::Min(G.GetLocalSize().X/1600.f,G.GetLocalSize().Y/900.f);FVector2D Old=Mouse;Mouse=(G.AbsoluteToLocal(E.GetScreenSpacePosition())-(G.GetLocalSize()-FVector2D(1600,900)*Scale)/2)/Scale;if(MapDragging&&Host->Model->Screen=="Map"){Host->MapPan+=Mouse-Old;return FReply::Handled();}return FReply::Unhandled();}
FReply UDungeonWidget::NativeOnMouseButtonDown(const FGeometry& G,const FPointerEvent& E){NativeOnMouseMove(G,E);if(Host->Model->Screen=="Map"&&Mouse.X>=70&&Mouse.X<=1180&&Mouse.Y>=140&&Mouse.Y<=770){if(E.GetEffectingButton()==EKeys::MiddleMouseButton){MapDragging=true;return FReply::Handled().CaptureMouse(TakeWidget());}if(E.GetEffectingButton()==EKeys::LeftMouseButton){auto M=Host->Model;auto S=M->State;int F=Host->MapFloor>=0?Host->MapFloor:S->Floor;int W=M->Floors[F].Rows[0].Len();FVector2D Focus=F==S->Floor?FVector2D(S->X+.5f,S->Y+.5f):FVector2D(W/2.f,M->Floors[F].Rows.Num()/2.f);FDungeonMapTransform T{FVector2D(625,455)-Focus*Host->MapZoom+Host->MapPan,Host->MapZoom,W};int C=T.CellAt(Mouse);auto& R=S->Floors[F];if(R.MemoryTiles.Contains(C)&&R.MemoryTiles[C]!="#"){if(R.Markers.Contains(C))R.Markers.Remove(C);else R.Markers.Add(C);}return FReply::Handled();}}
 if(E.GetEffectingButton()==EKeys::LeftMouseButton)for(int I=Hits.Num()-1;I>=0;--I)if(Hits[I].Bounds.IsInside(Mouse)){Host->Command(Hits[I].Id);return FReply::Handled();}return Host->Model->Screen=="Map"?FReply::Handled():FReply::Unhandled();}
FReply UDungeonWidget::NativeOnMouseButtonUp(const FGeometry& G,const FPointerEvent& E){if(MapDragging&&E.GetEffectingButton()==EKeys::MiddleMouseButton){MapDragging=false;return FReply::Handled().ReleaseMouseCapture();}return Super::NativeOnMouseButtonUp(G,E);}
FReply UDungeonWidget::NativeOnMouseWheel(const FGeometry& G,const FPointerEvent& E){if(Host->Model->Screen=="Map"){Host->Command(E.GetWheelDelta()>0?"MapZoom:1":"MapZoom:-1");return FReply::Handled();}return Super::NativeOnMouseWheel(G,E);}
'''+s[b:];p.write_text(s)
p=r/'DungeonCommands.cpp';s=p.read_text();a=s.index(' if(P[0]=="Sell"&&!');s=s[:a]+''' if(M->Screen=="Map"){
  if(Id=="MapCenter"){MapPan=FVector2D::ZeroVector;return;}
  if(P[0]=="MapZoom"){MapZoom=FMath::Clamp(MapZoom*(A>0?1.25f:.8f),16.f,80.f);return;}
  if(P[0]=="RadarRange"){M->RadarRange=FMath::Clamp(M->RadarRange+A,2,6);M->UpdateDetection();SavePreferences();return;}
  if(P[0]=="MapFloor"){int F=MapFloor>=0?MapFloor:S->Floor;for(int N=F+A;S->Floors.IsValidIndex(N);N+=A)if(!S->Floors[N].Seen.IsEmpty()){MapFloor=N;MapPan=FVector2D::ZeroVector;break;}return;}
 }
'''+s[a:];p.write_text(s)
p=r/'DungeonGame.cpp';s=p.read_text();s=s.replace('if(M->Screen=="Dungeon"&&!M->Combat&&Tween>=1)', 'if(M->Screen=="Dungeon"&&!M->Combat&&!ShowHistory&&Tween>=1&&!Pressed("Map"))');s=s.replace('if(Pressed("Map")&&!M->State->InTown)M->Screen="Map";', 'if(Pressed("Map")&&!M->State->InTown){M->Screen="Map";MapFloor=M->State->Floor;MapPan=FVector2D::ZeroVector;}');a=s.index(' if((M->Screen=="Town"');s=s[:a]+' if(M->Screen=="Map"&&Pressed("Map")){M->Screen="Dungeon";return;}\n'+s[a:]
s=s.replace(' if(M->WorldDirty){', ' M->TickDoors(Delta);DetectionClock+=Delta;if(DetectionClock>=.2f){DetectionClock=0;M->UpdateDetection();}TickAtmosphere(Delta);\n if(M->WorldDirty){');s=s.replace('Scenery.Empty();Billboards.Empty();LastX=-1;', 'Scenery.Empty();Billboards.Empty();DoorVisuals.Empty();Flames.Empty();LastX=-1;');a=s.index('  Mesh("FloorTile",P);');b=s.index('  if((T==\'C\'',a);s=s[:a]+s[b:];a=s.index('  if((X+Y*2)%5==0)');b=s.index('\n }\n // Soft player',a);s=s[:a]+s[b:];s=s.replace(' // Soft player lantern', ' BuildArchitecture();\n // Soft player lantern');s=s.replace('M->Screen=="Loading"','M->Screen=="Loading"',1)
# Detection range is a presentation preference, not a magic sensing ability.
s=s.replace(' GConfig->GetFloat(TEXT("DungeonPreferences"),TEXT("Master")', ' GConfig->GetInt(TEXT("DungeonPreferences"),TEXT("RadarRange"),Model->RadarRange,GGameUserSettingsIni);Model->RadarRange=FMath::Clamp(Model->RadarRange,2,6);\n GConfig->GetFloat(TEXT("DungeonPreferences"),TEXT("Master")');s=s.replace('void ADungeonController::SavePreferences(){','void ADungeonController::SavePreferences(){GConfig->SetInt(TEXT("DungeonPreferences"),TEXT("RadarRange"),Model->RadarRange,GGameUserSettingsIni);');p.write_text(s)
