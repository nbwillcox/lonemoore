#include "DungeonModel.h"

namespace {
const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
bool Door(TCHAR T){return T=='D'||T=='+'||T=='G'||T=='?';}
TCHAR At(const TArray<FString>& R,int C){int W=R[0].Len();return C>=0&&C<W*R.Num()?R[C/W][C%W]:TCHAR('#');}
}

void UDungeonModel::GenerateFloor(int32 F,int32 Seed){
 auto& D=Floors[F];auto& R=State->Floors[F];R=FFloorRecord();R.LayoutSeed=Seed;R.ContentVersion=2;
 D.Rows.Init(FString::ChrN(19,'#'),19);FRandomStream Rand(Seed);
 auto Room=[&](int X0,int Y0,int X1,int Y1,FString Purpose){for(int Y=Y0;Y<=Y1;++Y)for(int X=X0;X<=X1;++X){D.Rows[Y][X]='.';R.Rooms.Add(Y*19+X,Purpose);}};
 Room(2,2,7,5,D.RegionIndex==1?"Drainage junction":"Antechamber");
 Room(4,5,4,14,"Passage");Room(7,5,7,8,"Passage");
 Room(3,8,8,12,D.RegionIndex==1?"Cistern":D.RegionIndex==5?"Guard hall":"Ossuary");
 Room(2,14,6,16,D.RegionIndex==1?"Pump chamber":"Reliquary");
 Room(7,3,9,3,"Passage");Room(10,2,16,5,"Abandoned stores");
 Room(1,9,1,11,"Hidden burial");D.Rows[10][2]='?';D.Rows[10][1]='$';
 Room(10,8,16,14,D.RegionIndex>=7?"Ritual sanctuary":"Lower sanctuary");
 D.Rows[10][9]='D';D.Rows[3][9]='G';D.Rows[3][3]='S';D.Rows[4][2]='L';D.Rows[15][3+Rand.RandRange(0,2)]='K';
 D.Rows[4][15]='C';D.Rows[10][8]='E';D.Rows[8][4]='!';D.Rows[9][7]='T';D.Rows[4][6]='H';
 D.Rows[10][12]=D.Boss.IsEmpty()?'.':'B';D.Rows[13][15]=F==Floors.Num()-1?'A':'>';D.Rows[12][15]='V';
 if(RecruitForFloor(F)>=0)D.Rows[15][6]='R';
 // Keep each campaign floor's encounter count and supply opportunities.
 int EnemyCount=0,ChestCount=0;for(const auto& Row:BaseFloors[F].Rows)for(TCHAR T:Row){EnemyCount+=T=='E';ChestCount+=T=='C';}
 const FIntPoint EnemySites[]={FIntPoint(4,6),FIntPoint(7,7),FIntPoint(12,12),FIntPoint(4,14)};
 for(int I=1;I<EnemyCount&&I<=4;++I)D.Rows[EnemySites[I-1].Y][EnemySites[I-1].X]='E';
 const FIntPoint ChestSites[]={FIntPoint(5,16),FIntPoint(8,12),FIntPoint(16,5)};
 for(int I=1;I<ChestCount&&I<=3;++I)D.Rows[ChestSites[I-1].Y][ChestSites[I-1].X]='C';
 // A second loop stays entirely on the entrance side of the mandatory gate.
 if(D.RegionIndex%3!=1){Room(2,5,2,8,"Passage");D.Rows[8][2]='.';}
 // Mirror whole authored rooms, connections and semantics together, never punch random holes.
 if(Rand.RandRange(0,1)){for(auto& Row:D.Rows)for(int X=0;X<9;++X)Swap(Row[X],Row[18-X]);TMap<int,FString> Mirrored;for(auto& P:R.Rooms)Mirrored.Add((P.Key/19)*19+18-P.Key%19,P.Value);R.Rooms=MoveTemp(Mirrored);}
 R.Layout=D.Rows;BuildBoundaries(F);
}

void UDungeonModel::BuildBoundaries(int32 F,bool Legacy){
 if(State->Floors[F].ContentVersion==4){BuildModularBoundaries(F);return;}
 if(State->Floors[F].ContentVersion==3){BuildExpandedBoundaries(F);return;}
 auto& R=State->Floors[F];const auto& D=Floors[F];int W=D.Rows[0].Len(),H=D.Rows.Num();R.FloorId=FString::Printf(TEXT("floor_%02d"),F);R.Boundaries.Empty();R.ProtectedCells.Empty();
 int Exit=-1;for(int C=0;C<W*H;++C)if(At(D.Rows,C)=='>'||At(D.Rows,C)=='A')Exit=C;
 // New floors have an authored sanctuary behind D. Legacy layouts retain all cells;
 // their lower stair enclosure receives explicit sealed boundaries, not a teleport veto.
 if(!Legacy&&R.ContentVersion==2){for(auto& P:R.Rooms)if(P.Value=="Lower sanctuary"||P.Value=="Ritual sanctuary")R.ProtectedCells.Add(P.Key);}
 else if(Exit>=0){TArray<int> Q{Exit};R.ProtectedCells.Add(Exit);for(int I=0;I<Q.Num();++I){int C=Q[I];for(int Dir=0;Dir<4;++Dir){int X=C%W+DX[Dir],Y=C/W+DY[Dir],N=Y*W+X;TCHAR T=At(D.Rows,N);if(X<0||X>=W||Y<0||Y>=H||T=='#'||T=='D'||T=='K'||T=='L'||T=='S'||T=='R'||T=='B'||FMath::Abs(X-Exit%W)+FMath::Abs(Y-Exit/W)>3||R.ProtectedCells.Contains(N))continue;R.ProtectedCells.Add(N);Q.Add(N);}}}
 for(int Y=0;Y<H;++Y)for(int X=0;X<W;++X){int A=Y*W+X;if(At(D.Rows,A)=='#')continue;
  for(int Dir=0;Dir<4;++Dir){int NX=X+DX[Dir],NY=Y+DY[Dir];int B=NY*W+NX;if(NX<0||NX>=W||NY<0||NY>=H)B=-1-Dir-A*4;TCHAR T=At(D.Rows,B);if(T!='#'&&B<A)continue;
   FDungeonBoundary E;E.A=A;E.B=B;E.Id=R.FloorId+FString::Printf(TEXT(":edge:%d:%d"),FMath::Min(A,B),FMath::Max(A,B));E.Kind=T=='#'?"Wall":"Open";
   if(T!='#'){
    int Obj=Door(At(D.Rows,A))?A:Door(T)?B:-1;
    if(Obj>=0&&!(R.ContentVersion==2&&At(D.Rows,Obj)=='D')){TCHAR K=At(D.Rows,Obj);E.Kind=K=='?'?"Secret":K=='G'?"Bars":"Door";E.ObjectCell=Obj;E.DoorId=R.FloorId+FString::Printf(TEXT(":door:%d"),Obj);E.Requirement=K=='D'?D.Key:K=='G'?"switch":"";E.SeeThrough=K=='G';}
    if(R.ProtectedCells.Contains(A)!=R.ProtectedCells.Contains(B)){
     E.Kind="Door";E.DoorId=R.FloorId+":descent_gate";E.Requirement=D.Key;E.Mandatory=true;E.SeeThrough=false;
    }
   }
   if(!E.DoorId.IsEmpty()&&R.Opened.Contains(E.ObjectCell)){R.UnlockedDoors.AddUnique(E.DoorId);R.OpenDoors.AddUnique(E.DoorId);if(E.Kind=="Secret")R.Secrets.AddUnique(E.DoorId);}
   R.Boundaries.Add(E);
  }
 }
 R.Layout=D.Rows;if(!R.ContentVersion)R.ContentVersion=1;IndexBoundaries(F);
}

const FDungeonBoundary* UDungeonModel::Boundary(int F,int A,int B)const{
 if(!State->Floors.IsValidIndex(F))return nullptr;
 if(BoundaryIndices.IsValidIndex(F)){uint64 Key=(uint64(uint32(FMath::Min(A,B)))<<32)|uint32(FMath::Max(A,B));if(auto I=BoundaryIndices[F].Find(Key))if(State->Floors[F].Boundaries.IsValidIndex(*I)){const auto& E=State->Floors[F].Boundaries[*I];if((E.A==A&&E.B==B)||(E.A==B&&E.B==A))return &E;}}
 return State->Floors[F].Boundaries.FindByPredicate([&](const FDungeonBoundary& E){return(E.A==A&&E.B==B)||(E.A==B&&E.B==A);});
}
bool UDungeonModel::CanCross(int F,int A,int B,bool Sight)const{
 const auto* E=Boundary(F,A,B);if(!E)return false;if(E->Kind=="Wall")return false;
 if(E->Kind=="Pit")return Sight;if(E->Kind=="Open")return true;const auto& R=State->Floors[F];
 return R.OpenDoors.Contains(E->DoorId)||(Sight&&E->SeeThrough);
}
bool UDungeonModel::HasRequirement(int F,const FDungeonBoundary& E)const{
 if(E.Requirement=="guardian"){bool Found=false;int W=Floors[F].Rows[0].Len();for(int Y=0;Y<Floors[F].Rows.Num();++Y)for(int X=0;X<W;++X)if(Floors[F].Rows[Y][X]=='B'){Found=true;if(!State->Floors[F].Defeated.Contains(Y*W+X))return false;}return Found&&State->Bosses.Contains(F);}
 if(State->Floors[F].UnlockedDoors.Contains(E.DoorId)||E.Requirement.IsEmpty())return true;
 if(E.Requirement=="switch")return !State->Floors[F].Switches.IsEmpty();
 return State->Keys.Contains(E.Requirement)||State->Keys.Contains(FString::Printf(TEXT("Master Key %d"),Floors[F].RegionIndex));
}
bool UDungeonModel::OpenBoundary(const FDungeonBoundary& Edge){
 auto E=Edge;auto& R=State->Floors[State->Floor];if(E.Kind=="Pit"){Say("A deep drop. Find a bridge around it.");return false;}if(E.Kind=="Wall"||E.Kind=="Open")return false;
 if(R.OpenDoors.Contains(E.DoorId)||DoorOpening.Contains(E.DoorId))return true;
 if(!HasRequirement(State->Floor,E)){Say(E.Requirement=="guardian"?"This floor's guardian seals the stairs. Defeat the guardian to break the ward.":E.Requirement=="switch"?"A lever operates this iron gate.":"Locked. Requires "+E.Requirement+".");return true;}
 R.UnlockedDoors.AddUnique(E.DoorId);if(E.Kind=="Secret"){R.Secrets.AddUnique(E.DoorId);HuntProgress("Secret",2);}
 DoorOpening.Add(E.DoorId,0.f);WorldDirty=true;Reveal();Say(E.Kind=="Secret"?"The stone withdraws, revealing a hidden passage.":"The gate is opening.");return true;
}
void UDungeonModel::TickDoors(float Delta){
 if(!State->Floors.IsValidIndex(State->Floor))return;TArray<FString> Done;
 for(auto& P:DoorOpening){P.Value=FMath::Min(1.f,P.Value+Delta/.65f);if(P.Value>=1)Done.Add(P.Key);}
 for(auto& Id:Done){auto& R=State->Floors[State->Floor];R.OpenDoors.AddUnique(Id);for(auto& E:R.Boundaries)if(E.DoorId==Id&&E.ObjectCell>=0)R.Opened.AddUnique(E.ObjectCell);DoorOpening.Remove(Id);WorldDirty=true;Reveal();}
}

bool UDungeonModel::LineOfSight(int F,int From,int To)const{
 if(!Floors.IsValidIndex(F)||From<0||To<0)return false;int W=Floors[F].Rows[0].Len();int X=From%W,Y=From/W,TX=To%W,TY=To/W;
 const int SX=TX>X?1:-1,SY=TY>Y?1:-1;const float VX=FMath::Abs(TX-X),VY=FMath::Abs(TY-Y);float NextX=VX>0?.5f/VX:1e10f,NextY=VY>0?.5f/VY:1e10f;
 for(int Step=0;Step<W*Floors[F].Rows.Num();++Step){if(X==TX&&Y==TY)return true;int A=Y*W+X;
  if(FMath::IsNearlyEqual(NextX,NextY,.00001f)){int B=Y*W+X+SX,C=(Y+SY)*W+X,D=(Y+SY)*W+X+SX;if(!CanCross(F,A,B,true)||!CanCross(F,A,C,true)||!CanCross(F,B,D,true)||!CanCross(F,C,D,true))return false;X+=SX;Y+=SY;NextX+=1/VX;NextY+=1/VY;}
  else{bool Horizontal=NextX<NextY;int NX=X+(Horizontal?SX:0),NY=Y+(Horizontal?0:SY),B=NY*W+NX;if(!CanCross(F,A,B,true))return B==To&&(At(Floors[F].Rows,B)=='#'||Door(At(Floors[F].Rows,B)));X=NX;Y=NY;if(Horizontal)NextX+=1/VX;else NextY+=1/VY;}
 }
 return false;
}
void UDungeonModel::Reveal(){
 Visible.Empty();if(!State->Floors.IsValidIndex(State->Floor)||State->InTown)return;int F=State->Floor;auto& R=State->Floors[F];int W=Floors[F].Rows[0].Len(),From=Cell(State->X,State->Y);
 for(int Y=FMath::Max(0,State->Y-6);Y<FMath::Min(Floors[F].Rows.Num(),State->Y+7);++Y)for(int X=FMath::Max(0,State->X-6);X<FMath::Min(W,State->X+7);++X){int C=Y*W+X;if(FMath::Square(X-State->X)+FMath::Square(Y-State->Y)>36||!LineOfSight(F,From,C))continue;Visible.Add(C);R.Seen.AddUnique(C);TCHAR T=At(Floors[F].Rows,C);if(T=='?'&&!R.Opened.Contains(C))T='#';if(T=='E'||T=='B'||T=='H'||T=='m'||R.Taken.Contains(C))T='.';R.MemoryTiles.Add(C,FString::Chr(T));}
 for(const auto& E:R.Boundaries)if((Visible.Contains(E.A)&&R.MemoryTiles.FindRef(E.A)!="#")||(Visible.Contains(E.B)&&R.MemoryTiles.FindRef(E.B)!="#")){FString K=E.Kind;if(K=="Secret"&&!R.Secrets.Contains(E.DoorId))K="Wall";else if(!E.DoorId.IsEmpty()&&R.OpenDoors.Contains(E.DoorId))K="Open";R.MemoryEdges.Add(E.Id,K);}
 ++DiscoveryRevision;UpdateDetection();
}
void UDungeonModel::UpdateDetection(){
 DetectedEnemies.Empty();if(State->InTown||!State->Floors.IsValidIndex(State->Floor))return;int F=State->Floor,W=Floors[F].Rows[0].Len(),From=Cell(State->X,State->Y);
 for(int C:Visible){TCHAR T=At(Floors[F].Rows,C);if((T=='E'||T=='B'||T=='m'||(T=='H'&&HuntAt(C)>=0))&&!IsDefeated(C)&&FMath::Square(C%W-State->X)+FMath::Square(C/W-State->Y)<=RadarRange*RadarRange&&LineOfSight(F,From,C)){
  // A closed opaque door can itself be visible; its far side cannot expose an enemy.
  DetectedEnemies.Add(C);
 }}
}

bool UDungeonModel::ValidateFloor(int F,FString& Error)const{
 const auto& D=Floors[F];const auto& R=State->Floors[F];if(D.Rows.IsEmpty()||D.Rows[0].Len()<3){Error="Empty layout";return false;}int W=D.Rows[0].Len(),H=D.Rows.Num(),Start=-1,Exit=-1;
 for(int Y=0;Y<H;++Y){if(D.Rows[Y].Len()!=W){Error="Ragged layout";return false;}for(int X=0;X<W;++X){TCHAR T=D.Rows[Y][X];if((X==0||Y==0||X==W-1||Y==H-1)&&T!='#'){Error="Unsealed exterior";return false;}if(T=='S')Start=Y*W+X;if(T=='>'||T=='A')Exit=Y*W+X;}}
 if(Start<0||Exit<0){Error="Missing entrance or exit";return false;}
 TSet<FString> Ids;for(const auto& E:R.Boundaries){if(E.A<0||E.B<0||E.A>=W*H||E.B>=W*H||FMath::Abs(E.A%W-E.B%W)+FMath::Abs(E.A/W-E.B/W)!=1||Ids.Contains(E.Id)||!E.Id.StartsWith(R.FloorId+":edge:")){Error=R.FloorId+": invalid or duplicate boundary "+E.Id;return false;}Ids.Add(E.Id);}
 for(int Y=1;Y<H-1;++Y)for(int X=1;X<W-1;++X)if(D.Rows[Y][X]!='#')for(int Dir=0;Dir<4;++Dir)if(!Boundary(F,Y*W+X,(Y+DY[Dir])*W+X+DX[Dir])){Error=R.FloorId+": missing explicit boundary";return false;}
 if(R.ContentVersion==4)return ValidateModularFloor(F,Error);
 if(R.ContentVersion==3)return ValidateExpandedFloor(F,Error);
 // Fixed point over reachable cells and legal persistent actions, starting with no keys.
 auto Reach=[&](bool HoldRequirement,TSet<int>& Seen,TMap<int,int>& Parent){bool Key=false,Switch=false;TSet<FString> Unlocked;Seen.Add(Start);Parent.Add(Start,-1);bool Changed=true;while(Changed){Changed=false;for(int C:TArray<int>(Seen.Array())){TCHAR T=At(D.Rows,C);if(T=='K'&&!HoldRequirement&&!Key){Key=true;Changed=true;}if((T=='L'||T=='P')&&!Switch){Switch=true;Changed=true;}}
  for(const auto& E:R.Boundaries){if(E.Kind=="Wall")continue;bool Allowed=E.Kind=="Open"||Unlocked.Contains(E.DoorId)||E.Requirement.IsEmpty()||(E.Requirement=="switch"?Switch:Key);if(E.Mandatory&&HoldRequirement)Allowed=false;if(!Allowed)continue;if(Seen.Contains(E.A)||Seen.Contains(E.B)){if(!E.DoorId.IsEmpty())Unlocked.Add(E.DoorId);int A=Seen.Contains(E.A)?E.A:E.B,B=A==E.A?E.B:E.A;if(At(D.Rows,B)!='#'&&!Seen.Contains(B)){Seen.Add(B);Parent.Add(B,A);Changed=true;}}}
 }};
 TSet<int> Before,After;TMap<int,int> PB,PA;Reach(true,Before,PB);Reach(false,After,PA);
 if(Before.Contains(Exit)){FString Route;for(int C=Exit;C>=0;C=PB[C])Route=FString::FromInt(C)+" "+Route;Error=R.FloorId+":descent_gate bypass cells "+Route;return false;}
 bool KeyReachable=false;for(int C:Before)if(At(D.Rows,C)=='K')KeyReachable=true;
 if(!KeyReachable||!After.Contains(Exit)){Error=R.FloorId+": prerequisite dependency or unreachable protected stairs";return false;}
 // Optional switches may legally lie beyond the main key gate. After is still a
 // legal-action fixed point: switch gates remain closed until a lever is reached.
 for(const auto& E:R.Boundaries)if(E.Requirement=="switch"){bool Found=false;for(int C:After)if(At(D.Rows,C)=='L'||At(D.Rows,C)=='P')Found=true;if(!Found){Error=E.Id+": switch is behind its gate";return false;}}
 return true;
}

bool UDungeonModel::RestoreTopology(FString& Error){
 for(int F=0;F<Floors.Num();++F){auto& R=State->Floors[F];if(!R.Layout.IsEmpty())Floors[F].Rows=R.Layout;const auto& Rows=Floors[F].Rows;if(Rows.Num()<3||Rows.Num()>100||Rows[0].Len()<3||Rows[0].Len()>100){Error="Invalid saved layout dimensions";return false;}for(const auto& Row:Rows)if(Row.Len()!=Rows[0].Len()){Error="Ragged saved layout";return false;}if(R.ContentVersion>4){Error="Newer dungeon content version";return false;}if(R.Boundaries.IsEmpty())BuildBoundaries(F,true);IndexBoundaries(F);if(!ValidateFloor(F,Error))return false;
  RepairModularEncounters(F);
  if(R.MemoryTiles.IsEmpty()&&!R.Seen.IsEmpty()){for(int C:R.Seen)if(C>=0&&C<Rows.Num()*Rows[0].Len()){TCHAR T=At(Rows,C);if(T=='?'&&!R.Opened.Contains(C))T='#';if(T=='E'||T=='B'||T=='H'||T=='m'||R.Taken.Contains(C))T='.';R.MemoryTiles.Add(C,FString::Chr(T));}for(const auto& E:R.Boundaries)if(R.Seen.Contains(E.A)||R.Seen.Contains(E.B))R.MemoryEdges.Add(E.Id,E.Kind=="Secret"&&!R.Secrets.Contains(E.DoorId)?"Wall":R.OpenDoors.Contains(E.DoorId)?"Open":E.Kind);}
 }
 State->Schema=2;DoorOpening.Empty();Visible.Empty();DetectedEnemies.Empty();Reveal();return true;
}
