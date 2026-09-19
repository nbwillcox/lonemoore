#include "DungeonModel.h"
namespace {
const int StepX[]={0,1,0,-1},StepY[]={-1,0,1,0};
FString CompanionBark(const UDungeonModel& Model,int Floor,int Class){
 if(Class==Model.Floors[Floor].Recruit)return Model.Floors[Floor].RecruitBark;
 return Model.Classes.IsValidIndex(Class)?Model.Classes[Class].Name+TEXT(": I have watched this passage for too long. I will stand with you."):FString();
}
}
int32 UDungeonModel::Cell(int32 X,int32 Y)const{return Y*Floors[State->Floor].Rows[0].Len()+X;}
TCHAR UDungeonModel::Tile(int32 X,int32 Y)const{auto& R=Floors[State->Floor].Rows;return R.IsValidIndex(Y)&&X>=0&&X<R[Y].Len()?R[Y][X]:TCHAR('#');}
bool UDungeonModel::Walkable(int32 X,int32 Y)const{return Tile(X,Y)!='#'&&CanCross(State->Floor,Cell(State->X,State->Y),Cell(X,Y));}
bool UDungeonModel::IsDefeated(int32 C)const{auto& F=State->Floors[State->Floor];auto It=F.Defeated.Find(C);if(!It)return false;int32 W=Floors[State->Floor].Rows[0].Len();TCHAR T=Tile(C%W,C/W);return F.ContentVersion>=3||T=='B'||T=='m'||State->Transitions-*It<5;}
void UDungeonModel::EnterFloor(int32 FloorIndex,int32 Shrine){
 if(Combat||!Floors.IsValidIndex(FloorIndex))return;if(Shrine<0)UpgradeUnvisitedFloor(FloorIndex);RepairModularEncounters(FloorIndex);DoorOpening.Empty();State->Floor=FloorIndex;auto& D=Floors[FloorIndex];int32 W=D.Rows[0].Len();
 if(Shrine<0){for(int32 Y=0;Y<D.Rows.Num();++Y)for(int32 X=0;X<W;++X)if(D.Rows[Y][X]=='S')Shrine=Y*W+X;}
 State->X=Shrine%W;State->Y=Shrine/W;State->Facing=1;
 const auto& Record=State->Floors[FloorIndex];
 if(Record.ContentVersion==4&&D.Rows[State->Y][State->X]=='S'){
  if(const auto* Entrance=Record.RoomPlacements.FindByPredicate([&](const FRoomPlacement& P){return P.Id=="arrival_chamber"&&P.X==State->X&&P.Y==State->Y;}))
   for(int Direction:{1,2,0,3})if(Entrance->ActiveSockets&(1<<Direction)){State->Facing=Direction;break;}
 }
 State->InTown=false;State->Transitions++;State->Floors[FloorIndex].Shrines.AddUnique(Shrine);State->SelectedWaypoint=FloorIndex*10000+Shrine;Screen="Dungeon";WorldDirty=true;Reveal();Say(D.Name+". [W/S] step, [A/D] turn, [E] inspect. The shrine is a waypoint, not a healing fountain.");Save("Auto");
}
void UDungeonModel::Waypoint(int32 Packed){int32 F=Packed/10000,C=Packed%10000;if(Combat||!State->Floors.IsValidIndex(F)||!State->Floors[F].Shrines.Contains(C))return;EnterFloor(F,C);}
void UDungeonModel::Town(){if(Combat)return;State->InTown=true;Screen="Town";WorldDirty=true;Say(State->Postgame?"The old notices blacken. Hell Hunts now burn on the board.":"Lonemoore. Warm lamps, cold whispers. The darkness waits below.");Save("Auto");}
void UDungeonModel::Rotate(int32 Direction){if(Combat||Screen!="Dungeon")return;State->Facing=(State->Facing+Direction+4)%4;Reveal();}
bool UDungeonModel::Move(int32 Direction){
 if(Combat||Screen!="Dungeon"||(Direction!=1&&Direction!=-1))return false;int32 F=State->Facing;int32 X=State->X+StepX[F]*Direction,Y=State->Y+StepY[F]*Direction;
 if(!Walkable(X,Y)){Say(Tile(X,Y)=='#'?"Cold stone blocks the way.":"The way is closed. Face it and press E.");return false;}
 ReturnX=State->X;ReturnY=State->Y;int32 C=Cell(X,Y);TCHAR T=Tile(X,Y);
 // Encounters stop the camera on the approach tile so the enemy stays ahead.
 if(((T=='E'||T=='B'||T=='m')&&!IsDefeated(C))||(T=='H'&&HuntAt(C)>=0)){if(Direction<0)State->Facing=(State->Facing+2)%4;StartCombat(C,T!='E');return false;}
 State->X=X;State->Y=Y;Reveal();auto& R=State->Floors[State->Floor];
 if(T=='R'&&!R.Taken.Contains(C)){const int Companion=RecruitForFloor(State->Floor);Recruit(Companion);R.Taken.AddUnique(C);WorldDirty=true;Say(CompanionBark(*this,State->Floor,Companion));Save("Auto");}
 if(T=='T'&&!R.Taken.Contains(C)){R.Taken.Add(C);for(auto& H:State->Party)if(H.HP>0)H.HP=FMath::Max(0,H.HP-(6+Floors[State->Floor].Rank*2));Say("A scored pressure plate snaps down. Inspect suspicious floor tiles before stepping on them.");if(State->Hardcore&&State->Party[0].HP<=0)Defeat();else if(!State->Party.ContainsByPredicate([](const FPartyHero& H){return H.HP>0;}))Defeat();WorldDirty=true;}
 if(T=='P'){R.Switches.AddUnique(C);for(const auto& Edge:R.Boundaries)if(Edge.Requirement=="switch")OpenBoundary(Edge);WorldDirty=true;}

 if(T=='V'&&R.ExitCells.Contains(C))Say("A protected descent shrine. Press E to save and mark this return point.");
 else if(T=='S'||T=='V')Say("An ancient shrine. Press E to activate this waypoint.");
 else if(T=='>'||T=='A')Say(T=='A'?"Astra waits. Press E to approach.":"Stairs descend into darkness. Press E.");
 else if(T=='?'&&R.Opened.Contains(C))Say("A hidden passage, left behind by another age.");
 return true;
}
FString UDungeonModel::Context()const{if(Combat)return "Choose an action for the highlighted hero.";if(State->InTown)return "";int32 X=State->X+StepX[State->Facing],Y=State->Y+StepY[State->Facing];TCHAR T=Tile(X,Y),Here=Tile(State->X,State->Y);auto& R=State->Floors[State->Floor];int32 Ahead=Cell(X,Y);if(R.Taken.Contains(Ahead)||R.Opened.Contains(Ahead)||((T=='E'||T=='B'||T=='m')&&IsDefeated(Ahead)))T='.';if(Here=='V'&&R.ExitCells.Contains(Cell(State->X,State->Y)))return "[E] Save at descent shrine     [T] Teleport to Lonemoore";if(Here=='S'||Here=='V')return "[E] Activate shrine     [T] Teleport to Lonemoore";if(Here=='>'||Here=='A')return "[E] Continue the descent";if(T=='?')return "The mortar has a fine, straight seam. [E] Inspect";if(T=='D'||T=='+'||T=='G')return "[E] Inspect door / gate";if(T=='T'&&!R.Taken.Contains(Cell(X,Y)))return "Fine grooves cross the floor. [E] Inspect / disarm";if(T=='C'||T=='$'||T=='K')return "[E] Inspect / take";if(T=='L')return "[E] Pull lever";if(T=='!')return "[E] Read inscription";if(T=='R')return "[E] Speak";if(T=='E'||T=='B'||T=='m')return "A visible enemy guards the passage.";return "[W/S] Step    [A/D] Turn    [E] Inspect    [M] Map    [T] Town";}

FBagItem UDungeonModel::ChestLoot(int32 FloorIndex,int32 AtCell,bool Sanctuary)const{
 if(!State||!State->Floors.IsValidIndex(FloorIndex)||Items.IsEmpty())return FBagItem();
 // A local stream binds the sealed contents to this floor and chest. Combat,
 // party composition, saving/loading and failed inventory attempts cannot reroll it.
 const uint32 Seed=uint32(State->Floors[FloorIndex].LayoutSeed)^uint32(FloorIndex+1)*0x9e3779b9u^uint32(AtCell+1)*0x85ebca6bu^(Sanctuary?0xb7e15162u:0x243f6a88u);
 FRandomStream Random(static_cast<int32>(Seed));TArray<const FItemDef*> Pool;
 for(const auto& D:Items)if(!D.Id.IsEmpty()&&(!Sanctuary||!D.Slot.IsEmpty()))Pool.Add(&D);
 if(Pool.IsEmpty())return FBagItem();
 Pool.Sort([](const FItemDef& A,const FItemDef& B){return A.Id<B.Id;});
 const auto& Reward=*Pool[Random.RandRange(0,Pool.Num()-1)];
 const float Depth=FMath::Clamp(float(FloorIndex)/FMath::Max(1,Floors.Num()-1),0.f,1.f);
 const int Early[]={38,42,16,3,1},Deep[]={1,4,15,40,40};
 const float Draw=Random.FRand()*100.f;float Weight=0.f;int Quality=4;
 for(int Q=0;Q<5;++Q){Weight+=FMath::Lerp(float(Early[Q]),float(Deep[Q]),Depth);if(Draw<Weight){Quality=Q;break;}}
 if(Sanctuary)Quality=FMath::Max(2,Quality);
 const int Count=Reward.Stack>1?Random.RandRange(1,FMath::Min(3,Reward.Stack)):1;
 return FBagItem(Reward.Id,Count,Quality);
}

void UDungeonModel::Interact(){
 if(Combat||Screen!="Dungeon")return;auto& D=Floors[State->Floor];auto& R=State->Floors[State->Floor];int32 X=State->X+StepX[State->Facing],Y=State->Y+StepY[State->Facing];TCHAR Here=Tile(State->X,State->Y);
 if(Here=='S'||Here=='V'||Here=='>'||Here=='A'||((Here=='C'||Here=='$'||Here=='K'||Here=='L'||Here=='!'||Here=='R')&&!R.Taken.Contains(Cell(State->X,State->Y)))||(State->CorpseFloor==State->Floor&&State->CorpseCell==Cell(State->X,State->Y))){X=State->X;Y=State->Y;}
 int32 C=Cell(X,Y);TCHAR T=Tile(X,Y);
 if(X!=State->X||Y!=State->Y){const auto* E=Boundary(State->Floor,Cell(State->X,State->Y),C);if(!E||E->Kind=="Wall"){Say("Solid stone blocks the way.");return;}if(!CanCross(State->Floor,Cell(State->X,State->Y),C)){OpenBoundary(*E);return;}if(T=='>'||T=='A'){Say("Step onto the stairs to descend.");return;}}
 if(State->CorpseFloor==State->Floor&&State->CorpseCell==C&&!State->Corpse.IsEmpty()){while(State->Corpse.Num()>0){auto B=State->Corpse.Last();if(!AddItem(B))break;State->Corpse.Pop();}Say(State->Corpse.IsEmpty()?"You recover the last of the fallen party's belongings.":"Some belongings remain. Make room in your bags.");return;}
 if(T=='S'||T=='V'){
  const bool Descent=T=='V'&&R.ExitCells.Contains(C);
  if(Descent&&!CanDescend()){Say("The descent shrine is warded until the key and guardian seals are opened.");return;}
  R.Shrines.AddUnique(C);State->SelectedWaypoint=State->Floor*10000+C;
  const bool Saved=Save("Auto");
  Say(Descent?(Saved?"Game saved at the descent shrine. You can return here from town before continuing below.":"Descent waypoint activated, but saving failed. Try saving again before leaving."):(Saved?"Waypoint activated and game saved. Return here from the Dungeon Entrance in town.":"Waypoint activated, but saving failed. Try again before leaving."));return;
 }
 if((T=='>'||T=='A')&&!CanDescend()){Say("The descent is sealed. Defeat this floor's guardian and find "+D.Key+".");return;}
 if(T=='A'){if(!State->Postgame){Save("BeforeAstra");Screen="Choice";Say("Astra: You heard my call. Break the chains beside me, and this power is yours.");}else Say("The broken throne stands empty. Hell still breathes.");return;}
 if(T=='>'){
  if(!D.Boss.IsEmpty()&&!State->Bosses.Contains(State->Floor)){Say("The descent is sealed by this floor's guardian.");return;}
  const int Companion=RecruitForFloor(State->Floor);
  if(Companion>=0&&!State->Recruited.Contains(Companion)){Recruit(Companion);Say(CompanionBark(*this,State->Floor,Companion));}
  if(State->Floor+1<Floors.Num())EnterFloor(State->Floor+1);else Say("The deepest threshold has been reached.");return;
 }
 if(R.Taken.Contains(C)||R.Opened.Contains(C)){Say("You find nothing else here.");return;}
 if(T=='D'&&!R.Opened.Contains(C)&&!State->Keys.Contains(D.Key)&&!State->Keys.Contains(FString::Printf(TEXT("Master Key %d"),D.RegionIndex))){Say("Locked. Requires "+D.Key+".");return;}
 if(T=='G'&&!R.Opened.Contains(C)){Say("An iron gate. A lever or pressure plate must operate it.");return;}
 if(T=='D'||T=='+'||T=='?'){R.Opened.AddUnique(C);R.Seen.AddUnique(C);WorldDirty=true;Say(T=='?'?"The stone turns inward. You discovered a secret passage.":"The door opens.");if(T=='?')HuntProgress("Secret",2);return;}
 if(T=='L'){R.Switches.AddUnique(C);R.Taken.AddUnique(C);for(const auto& Edge:R.Boundaries)if(Edge.Requirement=="switch")OpenBoundary(Edge);Reveal();Say("A chain strains. The iron gate rises.");return;}

 if(R.Taken.Contains(C)){Say("You find nothing else here.");return;}
 if(T=='K'){State->Keys.AddUnique(D.Key);R.Taken.Add(C);WorldDirty=true;Reveal();Say("Obtained "+D.Key+". Keys do not take bag space.");return;}
 if(T=='T'){R.Taken.Add(C);WorldDirty=true;Say("You wedge the mechanism safely. Trap disarmed.");return;}
 if(T=='C'||T=='$'){
  const FBagItem Reward=ChestLoot(State->Floor,C,T=='$');
  if(AddItem(Reward)){R.Taken.Add(C);State->Gold+=T=='$'?80+D.Rank*25:15+D.Rank*6;WorldDirty=true;Reveal();Say(T=='$'?"A sealed sanctuary. Fine equipment and an ancient treasury are yours.":"Chest emptied. Its treasure and gold are added to the party bag.");HuntProgress("Relic",1);}
  else Say("The chest keeps its treasure safe. Make room in your bags, then return to collect it.");return;
 }
 if(T=='R'){const int Companion=RecruitForFloor(State->Floor);Recruit(Companion);R.Taken.Add(C);WorldDirty=true;Say(CompanionBark(*this,State->Floor,Companion));Save("Auto");return;}
 if(T=='!'){R.Taken.Add(C);Say(D.Lore);return;}
 Say("You find nothing else here.");
}
void UDungeonModel::GenerateOffers(){
 int32 Offers=0;for(auto H:State->Hunts)if(!H.Active)Offers++;while(Offers<4){
  FHuntRecord H;H.Serial=++State->HuntSerial;H.Hell=State->Postgame;H.TargetFloor=State->Postgame?FMath::Max(0,Floors.Num()-3):State->Floor;auto& D=Floors[H.TargetFloor];int32 Rank=D.Rank;
  H.Kind=H.Serial%6;if(H.Hell)H.Kind=H.Serial%4==0?0:3+H.Serial%3;
  H.Monster=D.Enemies[H.Serial%D.Enemies.Num()];if(H.Kind==5&&!D.Boss.IsEmpty())H.Monster=D.Boss;
  auto Enemy=EnemyDefs.FindByPredicate([&](const FEnemyDef& E){return E.Id==H.Monster;});H.Family=H.Hell?"Demon":H.Kind==1?"Relic":H.Kind==2?"Secret":Enemy?Enemy->Family:"Vermin";
  H.Goal=H.Kind?1:3+H.Serial%4;H.Name=H.Kind==1?"Recover a lost reliquary":H.Kind==2?"Find a forgotten passage":H.Kind>=3?(H.Kind==3?"Named target: ":H.Kind==4?"Champion hunt: ":"Rare boss hunt: ")+(Enemy?Enemy->Name:H.Monster):(H.Hell?"Hell Hunt: ":"Bounty: ")+H.Family;
  H.Gold=(H.Hell?500:45)+Rank*25+(H.Kind>=3?Rank*35:0);H.XP=(H.Hell?1200:50)+Rank*40+(H.Kind>=3?Rank*60:0);State->Hunts.Add(H);Offers++;
 }
}
int32 UDungeonModel::HuntAt(int32 AtCell)const{int32 W=Floors[State->Floor].Rows[0].Len();if(Tile(AtCell%W,AtCell/W)!='H')return -1;for(int32 I=0;I<State->Hunts.Num();++I){auto& H=State->Hunts[I];if(H.Active&&H.Kind>=3&&H.Progress<H.Goal&&H.TargetFloor==State->Floor)return I;}return -1;}
void UDungeonModel::HuntProgress(const FString& Family,int32 Kind){for(auto& H:State->Hunts)if(H.Active&&H.Kind==Kind&&H.Family==Family)H.Progress=FMath::Min(H.Goal,H.Progress+1);}
void UDungeonModel::AcceptHunt(int32 Index){if(!State->Hunts.IsValidIndex(Index)||State->Hunts[Index].Active)return;int32 Active=0;for(auto H:State->Hunts)if(H.Active)Active++;if(Active>=5){Say("You already have five active hunts.");return;}State->Hunts[Index].Active=true;Say("Hunt accepted. Progress is recorded in the journal [J].");GenerateOffers();Save("Auto");}
void UDungeonModel::DeclineHunt(int32 Index){if(!State->Hunts.IsValidIndex(Index)||State->Hunts[Index].Active)return;State->Hunts.RemoveAt(Index);GenerateOffers();Save("Auto");}
void UDungeonModel::ClaimHunt(int32 Index){if(!State->InTown||!State->Hunts.IsValidIndex(Index))return;auto H=State->Hunts[Index];if(!H.Active||H.Progress<H.Goal)return;State->Gold+=H.Gold;GiveXP(H.XP);if(H.Hell)AddItem(EquipmentLoot(Roll(4)==0?4:3));State->Hunts.RemoveAt(Index);GenerateOffers();Say("Hunt reward collected.");Save("Auto");}
void UDungeonModel::FinalChoice(bool SideWithAstra){if(Screen!="Choice")return;Save("BeforeAstra");int32 X=State->X,Y=State->Y;if(Tile(X,Y)!='A'){X+=StepX[State->Facing];Y+=StepY[State->Facing];}StartCombat(Cell(X,Y),true,SideWithAstra);}
void UDungeonModel::FinishCredits(){if(State->Ending==1){Screen="Stinger";Say("Below the ruined throne, a sliver of the portal opens. Hell remembers.");}else{Screen="Menu";}}
void UDungeonModel::BeginPostgame(){if(Screen!="Stinger"||State->Ending!=1)return;State->Postgame=true;State->Hunts.Empty();GenerateOffers();Town();Save("Postgame");}
