#include "DungeonRoomKit.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace DungeonRoomKit {
FIntPoint RotateCell(FIntPoint P,int32 Q){for(int I=0;I<(Q&3);++I)P=FIntPoint(-P.Y,P.X);return P;}
int32 RotateMask(int32 M,int32 Q){Q&=3;return ((M<<Q)|(M>>(4-Q)))&15;}
TCHAR Tile(const FRoomTemplate& T,int32 X,int32 Y,int32 Q){
 const auto P=RotateCell(FIntPoint(X-HalfTiles,Y-HalfTiles),(4-Q)&3);
 const int LX=P.X+HalfTiles,LY=P.Y+HalfTiles;
 return T.Rows.IsValidIndex(LY)&&LX>=0&&LX<T.Rows[LY].Len()?T.Rows[LY][LX]:TCHAR('#');
}
const TArray<FRoomTemplate>& Templates(){
 static TArray<FRoomTemplate> Rooms;static bool Loaded=false;if(Loaded)return Rooms;Loaded=true;
 FString Raw;TSharedPtr<FJsonObject> Root;
 if(!FFileHelper::LoadFileToString(Raw,*(FPaths::ProjectContentDir()/TEXT("Game/Data/room_kit.json")))||!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Raw),Root)||!Root.IsValid())return Rooms;
 const TArray<TSharedPtr<FJsonValue>>* Values=nullptr;if(!Root->TryGetArrayField(TEXT("templates"),Values))return Rooms;
 for(auto Value:*Values){auto O=Value->AsObject();if(!O.IsValid())continue;FRoomTemplate T;
  O->TryGetStringField(TEXT("id"),T.Id);O->TryGetStringField(TEXT("name"),T.Name);O->TryGetStringField(TEXT("mesh"),T.Mesh);
  O->TryGetNumberField(TEXT("socketMask"),T.SocketMask);O->TryGetNumberField(TEXT("ceilingCm"),T.CeilingCm);
  const TArray<TSharedPtr<FJsonValue>>* Rows=nullptr;if(O->TryGetArrayField(TEXT("rows"),Rows))for(auto R:*Rows)T.Rows.Add(R->AsString());
  auto Markers=[&](const TCHAR* Key,TArray<FRoomMarker>& Out){const TArray<TSharedPtr<FJsonValue>>* List=nullptr;if(!O->TryGetArrayField(Key,List))return;
   for(auto V:*List){auto M=V->AsObject();if(!M.IsValid())continue;FRoomMarker P;const TArray<TSharedPtr<FJsonValue>>* Pos=nullptr;
    if(M->TryGetArrayField(TEXT("position"),Pos)&&Pos->Num()==3)P.Position=FVector((*Pos)[0]->AsNumber(),(*Pos)[1]->AsNumber(),(*Pos)[2]->AsNumber());
    double Yaw=0;M->TryGetNumberField(TEXT("yaw"),Yaw);P.Yaw=float(Yaw);M->TryGetStringField(TEXT("kind"),P.Kind);M->TryGetNumberField(TEXT("variant"),P.Variant);Out.Add(P);
   }
  };Markers(TEXT("lights"),T.Lights);Markers(TEXT("props"),T.Props);Rooms.Add(MoveTemp(T));
 }
 return Rooms;
}
const FRoomTemplate* Find(const FString& Id){return Templates().FindByPredicate([&](const FRoomTemplate& T){return T.Id==Id;});}
bool Validate(FString& Error){
 const auto& Kit=Templates();if(Kit.Num()!=25){Error="Room kit must contain exactly 25 authored templates";return false;}
 TSet<FString> Ids;
 for(const auto& T:Kit){
  if(T.Id.IsEmpty()||Ids.Contains(T.Id)||!T.Mesh.StartsWith(TEXT("/Game/RoomKit/Meshes/"))||T.Rows.Num()!=TileCount||T.SocketMask<1||T.SocketMask>15||T.CeilingCm<500){Error="Invalid room template: "+T.Id;return false;}Ids.Add(T.Id);
  int Ground=0;for(int Y=0;Y<TileCount;++Y){if(T.Rows[Y].Len()!=TileCount){Error="Ragged room template: "+T.Id;return false;}
   for(int X=0;X<TileCount;++X){TCHAR C=T.Rows[Y][X];if(C!='#'&&C!='.'&&C!='~'){Error="Invalid room footprint character: "+T.Id;return false;}Ground+=C=='.';
    if(X==0||Y==0||X==6||Y==6){int Port=X==3&&Y==0?1:X==6&&Y==3?2:X==3&&Y==6?4:X==0&&Y==3?8:0;
     if(C=='.'&&(!Port||!(T.SocketMask&Port))){Error="Unsealed room perimeter: "+T.Id;return false;}
     if(Port&&(T.SocketMask&Port)&&C!='.'){Error="Socket has no walkable mouth: "+T.Id;return false;}
    }
   }
  }
  if(T.Rows[3][3]!='.'){Error="Room center must be walkable: "+T.Id;return false;}
  TSet<int> Seen;TArray<int> Q{24};Seen.Add(24);const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
  for(int I=0;I<Q.Num();++I)for(int D=0;D<4;++D){int X=Q[I]%7+DX[D],Y=Q[I]/7+DY[D],C=Y*7+X;if(X>=0&&Y>=0&&X<7&&Y<7&&T.Rows[Y][X]=='.'&&!Seen.Contains(C)){Seen.Add(C);Q.Add(C);}}
  if(Seen.Num()!=Ground){Error="Disconnected authored room footprint: "+T.Id;return false;}
 }
 for(const TCHAR* Id:{TEXT("arrival_chamber"),TEXT("twin_crypt"),TEXT("reliquary"),TEXT("boss_approach"),TEXT("boss_arena"),TEXT("sealed_descent")})if(!Ids.Contains(FString(Id))){Error="Missing required assembly template: "+FString(Id);return false;}
 const auto* Approach=Find(TEXT("boss_approach"));const auto* Arena=Find(TEXT("boss_arena"));const auto* Descent=Find(TEXT("sealed_descent"));
 if((Approach->SocketMask&5)!=5||(Arena->SocketMask&5)!=5||!(Descent->SocketMask&1)){Error="Guardian sequence requires north/south approach and arena sockets and a north descent entry";return false;}
 return true;
}
}
