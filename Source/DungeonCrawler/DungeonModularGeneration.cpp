#include "DungeonModel.h"
#include "DungeonRoomKit.h"

namespace {
// 64 exploration rooms plus four guardian-route sections: 34.6% fewer than
// the former 104, without shrinking the authored meshes or their walkways.
constexpr int Columns=8,Rows=8,Width=Columns*7+2,Height=Rows*7+30;
const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
bool Ground(TCHAR T){return T!='#'&&T!='~';}
int Bits(int M){int N=0;for(;M;M>>=1)N+=M&1;return N;}
int FloorArea(const FRoomTemplate& T){int N=0;for(const auto& Row:T.Rows)for(TCHAR C:Row)N+=C=='.';return N;}
int Direction(int A,int B){return B==A+1?1:B==A-1?3:B>A?2:0;}
FIntPoint Center(int I){return FIntPoint(4+(I%Columns)*7,4+(I/Columns)*7);}
}

void UDungeonModel::GenerateModularFloor(int32 F,int32 Seed){
 FString KitError;if(!DungeonRoomKit::Validate(KitError)){UE_LOG(LogTemp,Fatal,TEXT("Room kit rejected: %s"),*KitError);return;}
 // Preserve the shipping encounter identities while scaling floor pacing.
 GenerateFloor(F,Seed);int Original=0,Encounters=0;TArray<int> Sources;
 const int OriginalWidth=Floors[F].Rows[0].Len();
 for(int Y=0;Y<Floors[F].Rows.Num();++Y)for(int X=0;X<OriginalWidth;++X){const TCHAR T=Floors[F].Rows[Y][X];Original+=Ground(T);if(T=='E'){++Encounters;Sources.Add(Y*OriginalWidth+X);}}
 auto& D=Floors[F];auto& R=State->Floors[F];R=FFloorRecord();R.ContentVersion=4;R.LayoutSeed=Seed;R.OriginalArea=Original;R.OriginalEncounters=Encounters;
 D.Rows.Init(FString::ChrN(Width,'#'),Height);FRandomStream Rand(Seed);
 constexpr int Count=Columns*Rows,SecretNode=Columns-1,VaultNode=(Rows-1)*Columns;
 TArray<int> Masks,Groups;Masks.Init(0,Count);for(int I=0;I<Count;++I)Groups.Add(I);
 TArray<FIntPoint> Links;
 for(int I=0;I<Count;++I)for(int J:{I%Columns<Columns-1?I+1:-1,I/Columns<Rows-1?I+Columns:-1})if(J>=0){
  // Two authored side chambers have exactly one approach, making their gates real.
  if((I==SecretNode||J==SecretNode)&&!(I==SecretNode-1&&J==SecretNode))continue;
  if((I==VaultNode||J==VaultNode)&&!(I==VaultNode&&J==VaultNode+1))continue;
  Links.Add(FIntPoint(I,J));
 }
 for(int I=Links.Num()-1;I>0;--I)Links.Swap(I,Rand.RandRange(0,I));
 for(auto Link:Links){bool Join=Groups[Link.X]!=Groups[Link.Y];if(!Join&&Rand.FRand()>.18f)continue;
  if(Join){int From=Groups[Link.Y],To=Groups[Link.X];for(auto& G:Groups)if(G==From)G=To;}
  const int Dir=Direction(Link.X,Link.Y);Masks[Link.X]|=1<<Dir;Masks[Link.Y]|=1<<((Dir+2)%4);
 }
 TArray<int> Distances;Distances.Init(-1,Count);Distances[0]=0;TArray<int> Queue{0};
 for(int Q=0;Q<Queue.Num();++Q){int I=Queue[Q];for(int Dir=0;Dir<4;++Dir)if(Masks[I]&(1<<Dir)){int J=I+DY[Dir]*Columns+DX[Dir];if(Distances[J]<0){Distances[J]=Distances[I]+1;Queue.Add(J);}}}
 int BossRoot=(Rows-1)*Columns+1;for(int I=BossRoot;I<Count;++I)if(Distances[I]>Distances[BossRoot])BossRoot=I;
 Masks[BossRoot]|=4;
 TArray<const FRoomTemplate*> Chosen;Chosen.Init(nullptr,Count);TArray<int> Rotations;Rotations.Init(0,Count);
 auto Assign=[&](int I,const FRoomTemplate* T){TArray<int> Fits;for(int Q=0;Q<4;++Q)if((Masks[I]&DungeonRoomKit::RotateMask(T->SocketMask,Q))==Masks[I])Fits.Add(Q);if(Fits.IsEmpty())return false;Chosen[I]=T;Rotations[I]=Fits[Rand.RandRange(0,Fits.Num()-1)];return true;};
 Assign(0,DungeonRoomKit::Find(TEXT("arrival_chamber")));
 Assign(SecretNode,DungeonRoomKit::Find(TEXT("twin_crypt")));Assign(VaultNode,DungeonRoomKit::Find(TEXT("reliquary")));
 // Put the restrictive straight and turning modules on compatible graph sockets
 // first, then distribute the broad chambers. Every mesh remains at authored scale.
 TArray<const FRoomTemplate*> Catalog;
 for(const auto& T:DungeonRoomKit::Templates())if(T.Id!=TEXT("arrival_chamber")&&!T.Id.StartsWith(TEXT("boss_"))&&T.Id!=TEXT("sealed_descent"))Catalog.Add(&T);
 Catalog.StableSort([](const FRoomTemplate& A,const FRoomTemplate& B){return Bits(A.SocketMask)<Bits(B.SocketMask);});
 for(const auto* T:Catalog){int Best=-1,Cost=99,First=Rand.RandRange(0,Count-1);for(int Offset=0;Offset<Count;++Offset){int I=(Offset+F*7+First)%Count;if(Chosen[I])continue;
   for(int Q=0;Q<4;++Q)if((Masks[I]&DungeonRoomKit::RotateMask(T->SocketMask,Q))==Masks[I]){int Score=Bits(T->SocketMask)-Bits(Masks[I]);if(Score<Cost){Best=I;Cost=Score;}}
  }if(Best>=0)Assign(Best,T);
 }
 TArray<const FRoomTemplate*> Broad;
 for(auto T:Catalog)if(T->SocketMask==15&&FloorArea(*T)>=25)Broad.Add(T);
 if(Broad.IsEmpty()){UE_LOG(LogTemp,Fatal,TEXT("Room kit requires broad exploration chambers"));return;}
 for(int I=0;I<Count;++I)if(!Chosen[I])Assign(I,Broad[(I+F+Rand.RandRange(0,Broad.Num()-1))%Broad.Num()]);
 auto Place=[&](const FRoomTemplate* T,FIntPoint P,int Q,int Mask,const FString& Purpose,bool Protected,bool Exit){
  FRoomPlacement Module;Module.Id=T->Id;Module.X=P.X;Module.Y=P.Y;Module.Rotation=Q;Module.ActiveSockets=Mask;R.RoomPlacements.Add(Module);
  FExpandedRoom V;V.X=P.X;V.Y=P.Y;V.RadiusX=V.RadiusY=3;V.Ceiling=T->CeilingCm;V.Purpose=Purpose;R.Volumes.Add(V);
  for(int LY=0;LY<7;++LY)for(int LX=0;LX<7;++LX){const int X=P.X+LX-3,Y=P.Y+LY-3,C=Y*Width+X;const TCHAR Tile=DungeonRoomKit::Tile(*T,LX,LY,Q);D.Rows[Y][X]=Tile;
   if(Tile!='#')R.Rooms.Add(C,Purpose);if(Ground(Tile)){if(Protected)R.ProtectedCells.Add(C);if(Exit)R.ExitCells.Add(C);}
  }
 };
 for(int I=0;I<Count;++I)Place(Chosen[I],Center(I),Rotations[I],Masks[I],Chosen[I]->Name,false,false);
 const FIntPoint Root=Center(BossRoot);const FIntPoint Approach1(Root.X,Root.Y+7),Approach2(Root.X,Root.Y+14),Arena(Root.X,Root.Y+21),Descent(Root.X,Root.Y+28);
 Place(DungeonRoomKit::Find(TEXT("boss_approach")),Approach1,0,5,TEXT("Approach to guardian"),false,false);
 Place(DungeonRoomKit::Find(TEXT("boss_approach")),Approach2,0,5,TEXT("Approach to guardian"),false,false);
 Place(DungeonRoomKit::Find(TEXT("boss_arena")),Arena,0,5,TEXT("Guardian arena"),true,false);
 Place(DungeonRoomKit::Find(TEXT("sealed_descent")),Descent,0,1,TEXT("Sealed descent"),true,true);
 auto Set=[&](FIntPoint P,TCHAR Tile){D.Rows[P.Y][P.X]=Tile;};
 Set(Center(0),'S');Set(Center(SecretNode),'$');Set(Center(VaultNode),'$');
 Set(Center(SecretNode)+FIntPoint(-3,0),'?');Set(Center(VaultNode)+FIntPoint(3,0),'G');
 Set(Arena,D.Boss.IsEmpty()?'.':'B');Set(FIntPoint(Arena.X,Arena.Y-3),'D');Set(Descent,F==Floors.Num()-1?'A':'>');
 TSet<int> Reserved{0,SecretNode,VaultNode,BossRoot};
 auto ReserveFarthest=[&](){int Best=-1;for(int I=0;I<Count;++I)if(!Reserved.Contains(I)&&(Best<0||Distances[I]>Distances[Best]))Best=I;Reserved.Add(Best);return Best;};
 const int KeyNode=ReserveFarthest();Set(Center(KeyNode),'K');const int LeverNode=ReserveFarthest();Set(Center(LeverNode),'L');
 // Leave the first two room exits available for a safe recruit room and the
 // opening encounter. On an 8x8 grid Count/4 can otherwise be the only room
 // two exits from arrival, turning the entire opening route into sanctuaries.
 for(int Desired:{Count/4,Count/2,Count*3/4}){int I=Desired;while(Reserved.Contains(I)||Distances[I]<=2)I=(I+1)%Count;Reserved.Add(I);Set(Center(I),'V');}
 int HuntNode=1;while(Reserved.Contains(HuntNode))++HuntNode;Reserved.Add(HuntNode);Set(Center(HuntNode),'H');
 if(RecruitForFloor(F)>=0){int I=-1;for(int Candidate=1;Candidate<Count;++Candidate)if(!Reserved.Contains(Candidate)&&(I<0||Distances[Candidate]<Distances[I]))I=Candidate;Reserved.Add(I);Set(Center(I),'R');}
 auto LocalSpare=[&](FIntPoint P,TCHAR T){for(int Y=-2;Y<=2;++Y)for(int X=-2;X<=2;++X)if(D.Rows[P.Y+Y][P.X+X]=='.'){Set(P+FIntPoint(X,Y),T);return;}};
 LocalSpare(Center(0),'!');
 // This save/return shrine is one cell left and before the stairs, wholly
 // inside the exit chamber and behind the same key and guardian gates.
 Set(Descent+FIntPoint(-1,-1),'V');
 TArray<int> Sites;
 for(int I=0;I<Count;++I)if(!Reserved.Contains(I)){
  const auto P=Center(I);Sites.Add(P.Y*Width+P.X);
  for(int Y=-2;Y<=2;++Y)for(int X=-2;X<=2;++X)if((X||Y)&&D.Rows[P.Y+Y][P.X+X]=='.')Sites.Add((P.Y+Y)*Width+P.X+X);
 }
 for(int I=Sites.Num()-1;I>0;--I)Sites.Swap(I,Rand.RandRange(0,I));
 PopulateModularEncounters(F,Sources,false);
 TSet<int> SuppliedRooms;int Supply=0;
 for(int C:Sites)if(D.Rows[C/Width][C%Width]=='.'){
  const int Room=((C/Width-1)/7)*Columns+(C%Width-1)/7;if(SuppliedRooms.Contains(Room))continue;
  SuppliedRooms.Add(Room);D.Rows[C/Width][C%Width]=Supply%5==4?'T':'C';if(++Supply==20)break;
 }
 R.Layout=D.Rows;BuildModularBoundaries(F);
}

void UDungeonModel::BuildModularBoundaries(int32 F){
 BuildExpandedBoundaries(F);auto& R=State->Floors[F];const int W=Floors[F].Rows[0].Len();TMap<int,int> Owner;
 for(int I=0;I<R.RoomPlacements.Num();++I){const auto& P=R.RoomPlacements[I];for(int Y=-3;Y<=3;++Y)for(int X=-3;X<=3;++X)Owner.Add((P.Y+Y)*W+P.X+X,I);}
 for(auto& E:R.Boundaries){const int* A=Owner.Find(E.A);const int* B=Owner.Find(E.B);if(!A||!B)continue;
  // The interaction tile names the port, but only the seam owns its door mesh.
  // Legacy tile gates on both tile faces would create two successive archways.
  if(*A==*B){if(E.Kind==TEXT("Secret")||E.Kind==TEXT("Bars")){E.Kind="Open";E.Requirement.Empty();E.DoorId.Empty();E.ObjectCell=-1;E.SeeThrough=false;}continue;}
  const auto& PA=R.RoomPlacements[*A];const auto& PB=R.RoomPlacements[*B];const int Dir=E.B%W>E.A%W?1:E.B%W<E.A%W?3:E.B/W>E.A/W?2:0;
  if(!(PA.ActiveSockets&(1<<Dir))||!(PB.ActiveSockets&(1<<((Dir+2)%4)))){E.Kind="Wall";E.Requirement.Empty();E.DoorId.Empty();E.ObjectCell=-1;E.Mandatory=false;E.SeeThrough=false;}
 }IndexBoundaries(F);
}

bool UDungeonModel::ValidateModularFloor(int32 F,FString& Error)const{
 if(!DungeonRoomKit::Validate(Error))return false;const auto& D=Floors[F];const auto& R=State->Floors[F];const int W=D.Rows[0].Len(),H=D.Rows.Num();
 if(W!=Width||H!=Height||W*H>=10000||R.RoomPlacements.Num()!=Columns*Rows+4||R.ContentVersion!=4){Error="Invalid modular floor dimensions or placement count";return false;}
 TMap<int,int> Owner;TSet<FIntPoint> Centers;int Approaches=0,Arenas=0,Descents=0;
 for(int I=0;I<R.RoomPlacements.Num();++I){const auto& P=R.RoomPlacements[I];const auto* T=DungeonRoomKit::Find(P.Id);
  if(!T||P.Rotation<0||P.Rotation>3||P.ActiveSockets<1||P.ActiveSockets>15||(P.X-4)%7||(P.Y-4)%7||(P.ActiveSockets&DungeonRoomKit::RotateMask(T->SocketMask,P.Rotation))!=P.ActiveSockets||Centers.Contains(FIntPoint(P.X,P.Y))){Error="Invalid authored room placement: "+P.Id;return false;}Centers.Add(FIntPoint(P.X,P.Y));
  Approaches+=P.Id==TEXT("boss_approach");Arenas+=P.Id==TEXT("boss_arena");Descents+=P.Id==TEXT("sealed_descent");
  for(int LY=0;LY<7;++LY)for(int LX=0;LX<7;++LX){int X=P.X+LX-3,Y=P.Y+LY-3,C=Y*W+X;if(X<1||Y<1||X>=W-1||Y>=H-1||Owner.Contains(C)){Error="Overlapping or out-of-bounds authored rooms";return false;}Owner.Add(C,I);const TCHAR A=DungeonRoomKit::Tile(*T,LX,LY,P.Rotation),B=D.Rows[Y][X];if((A=='#'&&B!='#')||(A=='~'&&B!='~')||(A=='.'&&!Ground(B))){Error="Navigation no longer matches authored room shell: "+P.Id;return false;}}
 }
 for(int Y=0;Y<H;++Y)for(int X=0;X<W;++X)if(D.Rows[Y][X]!='#'&&!Owner.Contains(Y*W+X)){Error="Dungeon floor extends outside its authored room geometry";return false;}
 if(Approaches!=2||Arenas!=1||Descents!=1){Error="Missing dedicated guardian approach or sealed descent";return false;}
 const auto* Descent=R.RoomPlacements.FindByPredicate([](const FRoomPlacement& P){return P.Id==TEXT("sealed_descent");});
 const int Checkpoint=(Descent->Y-1)*W+Descent->X-1;
 if(D.Rows[Descent->Y-1][Descent->X-1]!='V'||!R.ExitCells.Contains(Checkpoint)||!R.ProtectedCells.Contains(Checkpoint)){Error="Missing protected descent save waypoint";return false;}
 for(const auto& P:R.RoomPlacements)for(int Dir=0;Dir<4;++Dir)if(P.ActiveSockets&(1<<Dir)){
  const auto* Neighbor=R.RoomPlacements.FindByPredicate([&](const FRoomPlacement& N){return N.X==P.X+DX[Dir]*7&&N.Y==P.Y+DY[Dir]*7;});
  if(!Neighbor||!(Neighbor->ActiveSockets&(1<<((Dir+2)%4)))){Error="Unpaired room socket: "+P.Id;return false;}
  const int A=(P.Y+DY[Dir]*3)*W+P.X+DX[Dir]*3,B=A+DY[Dir]*W+DX[Dir];const auto* E=Boundary(F,A,B);
  if(!E||E->Kind==TEXT("Wall")||E->Kind==TEXT("Pit")){Error="Active room socket is blocked: "+P.Id;return false;}
 }
 for(const auto& E:R.Boundaries){auto A=Owner.Find(E.A),B=Owner.Find(E.B);if(!A||!B||*A==*B)continue;const auto& P=R.RoomPlacements[*A];int Dir=E.B%W>E.A%W?1:E.B%W<E.A%W?3:E.B/W>E.A/W?2:0;if(!(P.ActiveSockets&(1<<Dir))&&E.Kind!=TEXT("Wall")){Error="Inactive room socket has a navigation bypass";return false;}}
 return ValidateExpandedFloor(F,Error);
}
