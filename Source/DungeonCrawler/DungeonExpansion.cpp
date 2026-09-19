#include "DungeonModel.h"

namespace {
constexpr int W=85,H=99;
const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
bool Ground(TCHAR T){return T!='#'&&T!='~';}
uint64 EdgeKey(int A,int B){return (uint64(uint32(FMath::Min(A,B)))<<32)|uint32(FMath::Max(A,B));}
}

void UDungeonModel::IndexBoundaries(int32 F){
 BoundaryIndices.SetNum(State->Floors.Num());auto& Index=BoundaryIndices[F];Index.Empty();
 const auto& Edges=State->Floors[F].Boundaries;
 for(int I=0;I<Edges.Num();++I)Index.Add(EdgeKey(Edges[I].A,Edges[I].B),I);
}

bool UDungeonModel::UpgradeUnvisitedFloor(int32 F){
 if(!State->Floors.IsValidIndex(F))return false;const auto& R=State->Floors[F];
 // Existing locations, corpse recovery, discoveries and completed actions keep their cells.
 if(R.ContentVersion>=4||(!State->InTown&&State->Floor==F)||State->CorpseFloor==F||State->Bosses.Contains(F)||!R.Seen.IsEmpty()||!R.MemoryTiles.IsEmpty()||!R.MemoryEdges.IsEmpty()||!R.Shrines.IsEmpty()||!R.Taken.IsEmpty()||!R.Opened.IsEmpty()||!R.Defeated.IsEmpty()||!R.OpenDoors.IsEmpty()||!R.UnlockedDoors.IsEmpty()||!R.Secrets.IsEmpty()||!R.Switches.IsEmpty()||!R.Markers.IsEmpty())return false;
 auto Before=R;auto Rows=Floors[F].Rows;int Seed=R.LayoutSeed?R.LayoutSeed:int32(uint32(State->RandomSeed)+uint32(F*7919));FString Error;
 for(int Attempt=0;Attempt<3;++Attempt){GenerateModularFloor(F,int32(uint32(Seed)+uint32(Attempt*104729)));if(ValidateFloor(F,Error))return true;}
 State->Floors[F]=MoveTemp(Before);Floors[F].Rows=MoveTemp(Rows);IndexBoundaries(F);UE_LOG(LogTemp,Error,TEXT("Unvisited expansion rejected, original retained: %s"),*Error);return false;
}

void UDungeonModel::GenerateExpandedFloor(int32 F,int32 Seed){
 // Measure the shipping generated floor, then build a separate versioned record.
 GenerateFloor(F,Seed);int Original=0,Encounters=0;TArray<int> Sources;int OldW=Floors[F].Rows[0].Len();for(int Y=0;Y<Floors[F].Rows.Num();++Y)for(int X=0;X<OldW;++X)if(Floors[F].Rows[Y][X]=='E')Sources.Add(Y*OldW+X);
 for(const auto& Row:Floors[F].Rows)for(TCHAR T:Row){Original+=Ground(T);Encounters+=T=='E';}
 auto& D=Floors[F];auto& R=State->Floors[F];R=FFloorRecord();R.ContentVersion=3;R.LayoutSeed=Seed;
 R.OriginalArea=Original;R.OriginalEncounters=Encounters;D.Rows.Init(FString::ChrN(W,'#'),H);FRandomStream Rand(Seed);
 auto Carve=[&](int X,int Y,const FString& Purpose){if(X>0&&X<W-1&&Y>0&&Y<H-1){D.Rows[Y][X]='.';R.Rooms.Add(Y*W+X,Purpose);}};
 auto Room=[&](int X,int Y,int RX,int RY,int Shape,FString Purpose,int Ceiling){
  FExpandedRoom V;V.X=X;V.Y=Y;V.RadiusX=RX;V.RadiusY=RY;V.Ceiling=Ceiling;V.Purpose=Purpose;R.Volumes.Add(V);
  for(int VY=-RY;VY<=RY;++VY)for(int VX=-RX;VX<=RX;++VX){
   bool Inside=Shape==0||(Shape==1?float(VX*VX)/(RX*RX)+float(VY*VY)/(RY*RY)<=1.15f:Shape==2?FMath::Abs(VX)<=RX/2||FMath::Abs(VY)<=RY/2:FMath::Abs(VX)+FMath::Abs(VY)<RX+RY-2);
   if(Inside)Carve(X+VX,Y+VY,Purpose);
  }
 };
 TArray<FIntPoint> Centers;
 for(int Y=0;Y<4;++Y)for(int X=0;X<4;++X){
  int I=Y*4+X,CX=11+X*20+Rand.RandRange(-1,1),CY=11+Y*20+Rand.RandRange(-1,1);
  Centers.Add(FIntPoint(CX,CY));
  static const TCHAR* Purposes[9][4]={
   {TEXT("Pilgrim nave"),TEXT("Bell chapel"),TEXT("Cloister"),TEXT("Vestry stores")},
   {TEXT("Pump chamber"),TEXT("Collapsed ruins"),TEXT("Cistern"),TEXT("Abandoned stores")},
   {TEXT("Burial gallery"),TEXT("Failed chapel"),TEXT("Ossuary"),TEXT("Memorial vault")},
   {TEXT("Scavenger camp"),TEXT("Root cavern"),TEXT("Thorn court"),TEXT("Salvage stores")},
   {TEXT("Vigil hall"),TEXT("Funerary chapel"),TEXT("Sepulchre"),TEXT("Reliquary")},
   {TEXT("Guard hall"),TEXT("Prison gallery"),TEXT("Armoury"),TEXT("Barracks stores")},
   {TEXT("Mineral cavern"),TEXT("Ancient ruins"),TEXT("Drake grotto"),TEXT("Buried sanctuary")},
   {TEXT("Ritual hall"),TEXT("Seal chamber"),TEXT("Witness shrine"),TEXT("Ashen archive")},
   {TEXT("Basalt court"),TEXT("Forge hall"),TEXT("Broken sanctuary"),TEXT("Chain gallery")}};
  const int Region=FMath::Clamp(D.RegionIndex,0,8);FString Purpose=I==5?TEXT("Great reservoir"):Purposes[Region][I%4];
  int RX=I==5?8:Rand.RandRange(4,6),RY=I==5?8:Rand.RandRange(4,6),Shape=I==5?0:Rand.RandRange(0,3);
  // Regional silhouettes: cruciform chapels, long burial halls, irregular cave lobes.
  if(I!=5&&Region!=1){if(Region==0||Region==4)Shape=I%3==0?2:Shape;else if(Region==2||Region==5){RX=I%2?4:7;RY=I%2?7:4;Shape=I%3?0:3;}else if(Region==3||Region==6){Shape=I%2?1:3;RX=Rand.RandRange(5,7);RY=Rand.RandRange(4,7);}else Shape=I%3==0?2:3;}
  Room(CX,CY,RX,RY,Shape,Purpose,I==5?2400:Rand.RandRange(9,16)*100);
 }
 // Random spanning tree guarantees connectivity. Extra links make real loops.
 TArray<FIntPoint> Links;for(int I=0;I<16;++I){if(I%4<3)Links.Add(FIntPoint(I,I+1));if(I/4<3)Links.Add(FIntPoint(I,I+4));}
 for(int I=Links.Num()-1;I>0;--I)Links.Swap(I,Rand.RandRange(0,I));
 TArray<int> Group;for(int I=0;I<16;++I)Group.Add(I);
 for(auto Link:Links){bool Join=Group[Link.X]!=Group[Link.Y];if(!Join&&Rand.FRand()>.45f)continue;
  if(Join){int Old=Group[Link.Y],New=Group[Link.X];for(auto& G:Group)if(G==Old)G=New;}
  FIntPoint A=Centers[Link.X],B=Centers[Link.Y];int Width=Rand.RandRange(0,1);bool Horizontal=Rand.RandRange(0,1)!=0;
  auto Dig=[&](bool AlongX){int& V=AlongX?A.X:A.Y;int Target=AlongX?B.X:B.Y;while(V!=Target){V+=V<Target?1:-1;for(int Offset=-Width;Offset<=Width;++Offset){int X=A.X+(AlongX?0:Offset),Y=A.Y+(AlongX?Offset:0);if(D.Rows[Y][X]=='#')Carve(X,Y,"Passage");}}};
  Dig(Horizontal);Dig(!Horizontal);
 }
 // Real holes beneath two broad crossing bridges; the rim remains connected.
 auto Reservoir=Centers[5];for(int Y=-6;Y<=6;++Y)for(int X=-6;X<=6;++X)
  if(FMath::Abs(X)>1&&FMath::Abs(Y)>1){D.Rows[Reservoir.Y+Y][Reservoir.X+X]='~';}
 // Grow irregular room margins until at least ten times the original walkable area.
 auto Area=[&](){int N=0;for(const auto& Row:D.Rows)for(TCHAR T:Row)N+=Ground(T);return N;};
 int CurrentArea=Area();
 for(int Pass=0;CurrentArea<Original*10&&Pass<5;++Pass)for(int I=0;I<16&&CurrentArea<Original*10;++I){auto V=R.Volumes[I];
  for(int Y=-8;Y<=8&&CurrentArea<Original*10;++Y)for(int X=-8;X<=8&&CurrentArea<Original*10;++X){int CX=V.X+X,CY=V.Y+Y;if(D.Rows[CY][CX]!='#')continue;
   bool Touch=false;for(int Dir=0;Dir<4;++Dir)Touch|=Ground(D.Rows[CY+DY[Dir]][CX+DX[Dir]]);if(Touch){Carve(CX,CY,V.Purpose);++CurrentArea;}
  }
 }
 auto End=Centers[14];for(int Y=End.Y;Y<=86;++Y)Carve(End.X,Y,"Approach to guardian");
 Room(End.X,89,7,3,3,"Guardian arena",1600);
 for(int Y=86;Y<=92;++Y)Carve(End.X,Y,"Guardian arena");
 // Single key-locked entry, then a second seal around the actual staircase.
 for(int Y=86;Y<=92;++Y)for(int X=End.X-7;X<=End.X+7;++X)if(Ground(D.Rows[Y][X]))R.ProtectedCells.Add(Y*W+X);
 D.Rows[85][End.X]='D';D.Rows[89][End.X]=D.Boss.IsEmpty()?'.':'B';
 for(int Y=93;Y<=96;++Y)for(int X=End.X-2;X<=End.X+2;++X){Carve(X,Y,"Sealed descent");R.ExitCells.Add(Y*W+X);R.ProtectedCells.Add(Y*W+X);}
 D.Rows[95][End.X]=F==Floors.Num()-1?'A':'>';D.Rows[94][End.X-1]='V';
 auto Start=Centers[0];D.Rows[Start.Y][Start.X]='S';D.Rows[Start.Y][Start.X+2]='!';
 auto Key=Centers[Rand.RandRange(8,11)];D.Rows[Key.Y][Key.X]='K';
 auto Lever=Centers[3];D.Rows[Lever.Y][Lever.X]='L';
 // Optional lever vault and secret alcove, separated from the guardian complex.
 for(int Y=Centers[7].Y;Y<=44;++Y)Carve(Centers[7].X,Y,"Passage");
 for(int X=Centers[7].X;X<=82;++X)Carve(X,44,"Passage");D.Rows[44][81]='?';D.Rows[44][82]='$';
 for(int X=Centers[3].X;X<=82;++X)Carve(X,Centers[3].Y,"Passage");
 D.Rows[Centers[3].Y][81]='G';D.Rows[Centers[3].Y][82]='$';D.Rows[Lever.Y][Lever.X]='L';
 for(int I:{2,6,10,13}){auto P=Centers[I];if(D.Rows[P.Y][P.X]=='.')D.Rows[P.Y][P.X]='V';}
 auto Hunt=Centers[4];D.Rows[Hunt.Y][Hunt.X]='H';
 if(RecruitForFloor(F)>=0){auto Companion=Centers[2];D.Rows[Companion.Y][Companion.X]='R';}
 // Distribute encounters away from entrances, waypoints and narrow bridges.
 TArray<int> Sites;for(int Y=2;Y<82;++Y)for(int X=2;X<W-2;++X){if(D.Rows[Y][X]!='.'||R.Rooms.FindRef(Y*W+X)=="Passage"||R.Rooms.FindRef(Y*W+X)=="Great reservoir")continue;
  bool Safe=true;for(int DY2=-2;DY2<=2;++DY2)for(int DX2=-2;DX2<=2;++DX2){int NX=X+DX2,NY=Y+DY2;if(NX<0||NY<0||NX>=W||NY>=H)continue;TCHAR T=D.Rows[NY][NX];if(T=='S'||T=='V'||T=='K'||T=='L')Safe=false;}
  if(Safe)Sites.Add(Y*W+X);
 }
 for(int I=Sites.Num()-1;I>0;--I)Sites.Swap(I,Rand.RandRange(0,I));
 int Placed=0;TArray<int> EnemyCells;for(int C:Sites){bool Near=false;for(int E:EnemyCells)Near|=FMath::Abs(C%W-E%W)+FMath::Abs(C/W-E/W)<5;if(Near)continue;D.Rows[C/W][C%W]='E';R.EncounterSources.Add(C,Sources[Placed%Sources.Num()]);EnemyCells.Add(C);if(++Placed==Encounters*10)break;}
 int Supply=0;for(int C:Sites)if(D.Rows[C/W][C%W]=='.'){D.Rows[C/W][C%W]=Supply%5==4?'T':'C';if(++Supply==30)break;}
 // Orient the whole floor differently without changing dependencies or room shapes.
 if(Rand.RandRange(0,1)){
  for(auto& Row:D.Rows)for(int X=0;X<W/2;++X)Swap(Row[X],Row[W-1-X]);
  auto Mirror=[&](int C){return (C/W)*W+W-1-C%W;};TMap<int,FString> Rooms;
  for(auto P:R.Rooms)Rooms.Add(Mirror(P.Key),P.Value);R.Rooms=MoveTemp(Rooms);
  TMap<int,int> MirroredSources;for(auto P:R.EncounterSources)MirroredSources.Add(Mirror(P.Key),P.Value);R.EncounterSources=MoveTemp(MirroredSources);
  for(auto& C:R.ProtectedCells)C=Mirror(C);for(auto& C:R.ExitCells)C=Mirror(C);for(auto& V:R.Volumes)V.X=W-1-V.X;
 }
 R.Layout=D.Rows;BuildExpandedBoundaries(F);
}

void UDungeonModel::BuildExpandedBoundaries(int32 F){
 auto& R=State->Floors[F];auto& Rows=Floors[F].Rows;int Width=Rows[0].Len();R.FloorId=FString::Printf(TEXT("floor_%02d"),F);R.Boundaries.Empty();
 for(int Y=1;Y<Rows.Num()-1;++Y)for(int X=1;X<Width-1;++X){TCHAR T=Rows[Y][X];if(T=='#')continue;int A=Y*Width+X;
  for(int Dir=0;Dir<4;++Dir){int NX=X+DX[Dir],NY=Y+DY[Dir],B=NY*Width+NX;TCHAR U=Rows[NY][NX];if(U!='#'&&B<A)continue;
   FDungeonBoundary E;E.A=A;E.B=B;E.Id=R.FloorId+FString::Printf(TEXT(":edge:%d:%d"),FMath::Min(A,B),FMath::Max(A,B));E.Kind=U=='#'?"Wall":T=='~'||U=='~'?"Pit":"Open";E.SeeThrough=E.Kind=="Pit";
   if(Ground(T)&&Ground(U)){
    int Object=T=='?'||T=='G'?A:U=='?'||U=='G'?B:-1;
    if(Object>=0){TCHAR K=Rows[Object/Width][Object%Width];E.Kind=K=='?'?"Secret":"Bars";E.ObjectCell=Object;E.DoorId=R.FloorId+FString::Printf(TEXT(":door:%d"),Object);E.Requirement=K=='G'?"switch":"";E.SeeThrough=K=='G';}
    if(R.ProtectedCells.Contains(A)!=R.ProtectedCells.Contains(B)){E.Kind="Door";E.DoorId=R.FloorId+":descent_gate";E.Requirement=Floors[F].Key;E.Mandatory=true;E.SeeThrough=false;}
    if(R.ExitCells.Contains(A)!=R.ExitCells.Contains(B)){E.Kind="Door";E.DoorId=R.FloorId+":guardian_seal";E.Requirement=Floors[F].Boss.IsEmpty()?Floors[F].Key:FString("guardian");E.Mandatory=true;E.SeeThrough=false;}
   }R.Boundaries.Add(E);
  }
 }R.Layout=Rows;IndexBoundaries(F);
}

bool UDungeonModel::ValidateExpandedFloor(int32 F,FString& Error)const{
 const auto& D=Floors[F];const auto& R=State->Floors[F];int Width=D.Rows[0].Len(),Start=-1,Exit=-1,Boss=-1,Area=0,EnemyTotal=0,Starts=0,Exits=0,Bosses=0,Keys=0;
 int Recruits=0,Finals=0;bool HasBoss=!D.Boss.IsEmpty();
 for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<Width;++X){TCHAR T=D.Rows[Y][X];int C=Y*Width+X;Area+=Ground(T);EnemyTotal+=T=='E';if(T=='S'){Start=C;++Starts;}if(T=='>'||T=='A'){Exit=C;++Exits;}if(T=='B'){Boss=C;++Bosses;}Keys+=T=='K';Recruits+=T=='R';Finals+=T=='A';}
 const bool InvalidScale=R.ContentVersion==4?(Area*2<R.OriginalArea*13||EnemyTotal<R.OriginalEncounters*7):(Area<R.OriginalArea*10||EnemyTotal!=R.OriginalEncounters*10);
 if(Starts!=1||Exits!=1||Bosses!=int(HasBoss)||Keys!=1||Recruits!=int(RecruitForFloor(F)>=0)||Finals!=int(F==Floors.Num()-1)||InvalidScale){Error="Expanded floor objective count or version-specific size/density target failed";return false;}
 auto Reach=[&](bool AllowKey,bool AllowBoss,bool AllowSwitch){TSet<int> Seen;Seen.Add(Start);bool Key=false,Guardian=false,Switch=false,Changed=true;
  while(Changed){Changed=false;for(int C:TArray<int>(Seen.Array())){TCHAR T=D.Rows[C/Width][C%Width];if(T=='K'&&AllowKey&&!Key){Key=true;Changed=true;}if(T=='B'&&AllowBoss&&!Guardian){Guardian=true;Changed=true;}if(T=='L'&&AllowSwitch&&!Switch){Switch=true;Changed=true;}}
   TArray<int> Q=Seen.Array();for(int I=0;I<Q.Num();++I){int C=Q[I];for(int Dir=0;Dir<4;++Dir){int N=C+DY[Dir]*Width+DX[Dir];auto E=Boundary(F,C,N);if(!E||E->Kind=="Wall"||E->Kind=="Pit"||Seen.Contains(N)||!Ground(D.Rows[N/Width][N%Width]))continue;
    bool Allowed=E->Requirement.IsEmpty()||(E->Requirement=="guardian"?Guardian:E->Requirement=="switch"?Switch:Key);
    if(Allowed){Seen.Add(N);Q.Add(N);Changed=true;}
   }}
  }return Seen;
 };
 auto NoKey=Reach(false,true,true),NoBoss=Reach(true,false,true),All=Reach(true,true,true),NoSwitch=Reach(true,true,false);
 if(R.ContentVersion==4)for(int C:R.ExitCells)if(D.Rows[C/Width][C%Width]=='V'&&(NoKey.Contains(C)||(HasBoss&&NoBoss.Contains(C))||!All.Contains(C))){Error="Descent save waypoint bypasses key/guardian gating or is unreachable";return false;}
 if(NoKey.Contains(Boss)||NoKey.Contains(Exit)||(HasBoss&&(NoBoss.Contains(Exit)||!NoBoss.Contains(Boss)))||!All.Contains(Exit)||All.Num()!=Area){Error=FString::Printf(TEXT("Expanded reachability: no-key boss=%d exit=%d; no-boss exit=%d boss=%d; all exit=%d reached=%d area=%d; missing:"),NoKey.Contains(Boss),NoKey.Contains(Exit),NoBoss.Contains(Exit),NoBoss.Contains(Boss),All.Contains(Exit),All.Num(),Area);for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<Width;++X)if(Ground(D.Rows[Y][X])&&!All.Contains(Y*Width+X))Error+=FString::Printf(TEXT(" (%d,%d,%c)"),X,Y,D.Rows[Y][X]);return false;}
 for(const auto& E:R.Boundaries)if(E.Requirement=="switch"&&NoSwitch.Contains(E.A)&&NoSwitch.Contains(E.B)){Error="Optional lever vault can be bypassed";return false;}
 return true;
}
