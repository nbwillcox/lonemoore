#include "DungeonModel.h"

namespace {
const int DX[]={0,1,0,-1},DY[]={-1,0,1,0};
struct FEncounterRoom {int32 Room=0,Cell=0,Tie=0;};
}

int32 UDungeonModel::PopulateModularEncounters(int32 F,const TArray<int32>& OriginalSources,bool ExistingFloor){
 auto& R=State->Floors[F];auto& Rows=Floors[F].Rows;
 if(R.ContentVersion!=4||R.RoomPlacements.IsEmpty()||Rows.IsEmpty()||OriginalSources.IsEmpty())return 0;
 if(ExistingFloor&&R.EncounterRevision>=1)return 0;
 // One-time additive migration. Completed floors and corpse recovery are immutable.
 R.EncounterRevision=1;
 if(ExistingFloor&&(State->Bosses.Contains(F)||State->CorpseFloor==F||State->Ending!=0||State->Postgame||R.OpenDoors.Contains(R.FloorId+TEXT(":guardian_seal"))))return 0;
 TArray<int32> Sources;for(int Source:OriginalSources)Sources.AddUnique(Source);Sources.Sort();
 const int W=Rows[0].Len(),Count=R.RoomPlacements.Num();TMap<FIntPoint,int32> Centers;TMap<int32,int32> Owner;
 for(int I=0;I<Count;++I){const auto& P=R.RoomPlacements[I];Centers.Add(FIntPoint(P.X,P.Y),I);for(int Y=-3;Y<=3;++Y)for(int X=-3;X<=3;++X)Owner.Add((P.Y+Y)*W+P.X+X,I);}
 TArray<TArray<int32>> Neighbors;Neighbors.SetNum(Count);int StartRoom=0;
 for(int I=0;I<Count;++I){const auto& P=R.RoomPlacements[I];if(Rows[P.Y][P.X]=='S')StartRoom=I;for(int D=0;D<4;++D)if(P.ActiveSockets&(1<<D))if(auto N=Centers.Find(FIntPoint(P.X+DX[D]*7,P.Y+DY[D]*7)))Neighbors[I].Add(*N);}
 auto GraphDistances=[&](const TArray<int32>& Starts){TArray<int32> Dist;Dist.Init(MAX_int32/4,Count);TArray<int32> Q;for(int I:Starts){Dist[I]=0;Q.AddUnique(I);}for(int I=0;I<Q.Num();++I)for(int N:Neighbors[Q[I]])if(Dist[N]>Dist[Q[I]]+1){Dist[N]=Dist[Q[I]]+1;Q.Add(N);}return Dist;};
 const auto EntranceDistance=GraphDistances(TArray<int32>{StartRoom});
 TSet<int32> Occupied,Visited;TMap<int32,int32> SourceCopies;int ExistingSites=0;
 for(int Y=0;Y<Rows.Num();++Y)for(int X=0;X<W;++X)if(Rows[Y][X]=='E'){int C=Y*W+X;++ExistingSites;if(auto I=Owner.Find(C))Occupied.Add(*I);if(auto Source=R.EncounterSources.Find(C))++SourceCopies.FindOrAdd(*Source);}
 auto ProtectCell=[&](int C){if(auto I=Owner.Find(C))Visited.Add(*I);};
 if(ExistingFloor){
  for(int C:R.Seen)ProtectCell(C);for(auto Entry:R.MemoryTiles)ProtectCell(Entry.Key);for(auto Entry:R.Defeated)ProtectCell(Entry.Key);
  for(int C:R.Taken)ProtectCell(C);for(int C:R.Opened)ProtectCell(C);for(int C:R.Shrines)ProtectCell(C);for(int C:R.Markers)ProtectCell(C);for(int C:R.Switches)ProtectCell(C);
  for(const auto& E:R.Boundaries)if(R.MemoryEdges.Contains(E.Id)||(!E.DoorId.IsEmpty()&&(R.OpenDoors.Contains(E.DoorId)||R.UnlockedDoors.Contains(E.DoorId)))){ProtectCell(E.A);ProtectCell(E.B);}
  if(!State->InTown&&State->Floor==F)for(int Y=State->Y-6;Y<=State->Y+6;++Y)for(int X=State->X-6;X<=State->X+6;++X)if(X>=0&&Y>=0&&X<W&&Y<Rows.Num())ProtectCell(Y*W+X);
 }
 FRandomStream Random(int32(uint32(R.LayoutSeed)^0x6154e93bu));TArray<FEncounterRoom> Candidates;int OrdinaryRooms=0;
 for(int I=0;I<Count;++I){const auto& P=R.RoomPlacements[I];if(P.Id.StartsWith(TEXT("boss_"))||P.Id==TEXT("sealed_descent"))continue;
  bool Arrival=false,Sanctuary=false;for(int Y=-3;Y<=3;++Y)for(int X=-3;X<=3;++X){const TCHAR T=Rows[P.Y+Y][P.X+X];Arrival|=T=='S';Sanctuary|=T=='S'||T=='V'||T=='R'||T=='$'||T=='B'||T=='>'||T=='A';}
  if(Arrival)continue;++OrdinaryRooms;if(Sanctuary||Occupied.Contains(I)||(ExistingFloor&&Visited.Contains(I)))continue;
  int EntryDirection=-1;for(int D=0;D<4;++D)if(P.ActiveSockets&(1<<D))if(auto N=Centers.Find(FIntPoint(P.X+DX[D]*7,P.Y+DY[D]*7)))if(EntranceDistance[*N]<EntranceDistance[I]){EntryDirection=D;break;}
  int Cell=-1,Best=MAX_int32;for(int Y=-2;Y<=2;++Y)for(int X=-2;X<=2;++X){int C=(P.Y+Y)*W+P.X+X;if(Rows[P.Y+Y][P.X+X]!='.'||R.Defeated.Contains(C)||R.Taken.Contains(C))continue;
   const int Score=(FMath::Abs(X)+FMath::Abs(Y))*10-(EntryDirection<0?0:X*DX[EntryDirection]+Y*DY[EntryDirection]);if(Score<Best){Best=Score;Cell=C;}
  }
  if(Cell>=0)Candidates.Add({I,Cell,Random.RandRange(0,1000000)});
 }
 // Keep the established 70% room density as the floor shrinks. Seven copies
 // per original roster scale the former tenfold minimum to the smaller kit.
 const int CoverageTarget=FMath::CeilToInt(OrdinaryRooms*.70f);
 const int NeededSites=FMath::Max(R.OriginalEncounters*7,ExistingSites+FMath::Max(0,CoverageTarget-Occupied.Num()));
 const int EncounterTarget=FMath::DivideAndRoundUp(NeededSites,Sources.Num())*Sources.Num();
 int Added=0;
 auto Add=[&](int Index){const auto Candidate=Candidates[Index];Candidates.RemoveAt(Index);int Source=Sources[0];for(int S:Sources)if(SourceCopies.FindRef(S)<SourceCopies.FindRef(Source))Source=S;
  Rows[Candidate.Cell/W][Candidate.Cell%W]='E';R.EncounterSources.Add(Candidate.Cell,Source);++SourceCopies.FindOrAdd(Source);Occupied.Add(Candidate.Room);++Added;
 };
 // Put the first regular fight on the immediate exploration route, while the
 // entrance, recruited companions and activated shrine rooms remain safe.
 if(!ExistingFloor&&!Candidates.IsEmpty()){int Best=0;for(int I=1;I<Candidates.Num();++I)if(EntranceDistance[Candidates[I].Room]<EntranceDistance[Candidates[Best].Room]||(EntranceDistance[Candidates[I].Room]==EntranceDistance[Candidates[Best].Room]&&Candidates[I].Tie<Candidates[Best].Tie))Best=I;Add(Best);}
 while(ExistingSites+Added<EncounterTarget&&!Candidates.IsEmpty()){
  const auto DistanceFromFights=GraphDistances(Occupied.Array());int Best=0;
  for(int I=1;I<Candidates.Num();++I){const auto& A=Candidates[I];const auto& B=Candidates[Best];if(DistanceFromFights[A.Room]>DistanceFromFights[B.Room]||(DistanceFromFights[A.Room]==DistanceFromFights[B.Room]&&A.Tie<B.Tie))Best=I;}Add(Best);
 }
 R.Layout=Rows;
 UE_LOG(LogTemp,Display,TEXT("ROOM_KIT_ENCOUNTERS floor=%d existing=%d added=%d occupied_rooms=%d ordinary_rooms=%d repair=%d"),F,ExistingSites,Added,Occupied.Num(),OrdinaryRooms,ExistingFloor);
 return Added;
}

bool UDungeonModel::RepairModularEncounters(int32 F){
 if(!State->Floors.IsValidIndex(F))return false;const auto& R=State->Floors[F];if(R.ContentVersion!=4||R.EncounterRevision>=1)return false;
 TArray<int32> Sources;for(auto Pair:R.EncounterSources)Sources.AddUnique(Pair.Value);return PopulateModularEncounters(F,Sources,true)>0;
}
