#include "DungeonModel.h"
#include "DungeonStatusEffects.h"
#include "DungeonCombatAudio.h"
#include "DungeonCombatVFX.h"
#include "Kismet/GameplayStatics.h"

void UDungeonModel::CombatFeedback(const FString& Cue,int32 Recipient,bool Hero,bool Sound){
 CombatEffects.Add({Cue,Recipient,Hero,0.f});if(Sound)CombatCues.Add(Cue);
}

void UDungeonModel::StartCombat(int32 AtCell,bool Boss,bool Betray){
 if(Combat||State->Party.IsEmpty())return;CombatEffects.Empty();CombatCues.Empty();EncounterHunt=HuntAt(AtCell);Combat=true;BossFight=Boss||EncounterHunt>=0;Boss=BossFight;Betrayal=Betray;EncounterCell=AtCell;Enemies.Empty();Round=1;Target=0;SelectedHero=0;Screen="Dungeon";auto& D=Floors[State->Floor];int32 Rank=D.Rank+(State->Postgame?State->Party[0].Level/2:0);
 if(Betray){
  for(int32 I=1;I<State->Party.Num();++I){auto H=State->Party[I];if(H.HP<=0)continue;FEnemyUnit E;E.Id="companion";E.Name=HeroName(H);E.Art=Classes[H.Class].Art;E.Family="Companion";E.MaxHP=MaxHP(H);E.HP=E.MaxHP;E.Attack=Attack(H);E.Speed=H.Stats[1];E.Type=H.Class==3||H.Class==5?"Holy":"Physical";Enemies.Add(E);}
  auto& Hero=State->Party[0];Hero.HP=MaxHP(Hero)*4;Hero.MP=MaxMP(Hero)*3;Say("Your companions refuse. Nephilim power answers your betrayal.");
 }else{
  FString BossId=EncounterHunt>=0?State->Hunts[EncounterHunt].Monster:D.Boss;int32 Width=D.Rows[0].Len();if(Tile(AtCell%Width,AtCell/Width)=='A')BossId="astra";
  auto Roster=EncounterRoster(AtCell,Boss);int32 Count=Roster.Num();
  for(int32 I=0;I<Count;++I){FString Id=Boss?BossId:Roster[I];auto P=EnemyDefs.FindByPredicate([&](const FEnemyDef& E){return E.Id==Id;});if(!P)P=&EnemyDefs[0];FEnemyUnit E;E.Id=P->Id;E.Name=P->Name;E.Art=P->Art;E.Family=P->Family;E.Type=P->Type;E.Inflict=P->Status;E.Resist=P->Resist;E.MaxHP=FMath::RoundToInt((14+Rank*10)*P->HP*(Boss?1.3f+State->Party.Num()*.25f:1.f));E.HP=E.MaxHP;E.Attack=FMath::RoundToInt((4+Rank*3)*P->Attack);E.Speed=FMath::RoundToInt((4+Rank*.7f)*P->Speed);Enemies.Add(E);}
  for(auto& E:Enemies)if(E.Id=="astra"){E.MaxHP*=3;E.HP=E.MaxHP;E.Attack=FMath::RoundToInt(E.Attack*1.8f);}
  if(EncounterHunt>=0){for(auto& E:Enemies){E.Name="Marked "+E.Name;E.MaxHP*=State->Hunts[EncounterHunt].Kind-1;E.HP=E.MaxHP;}Say("Your quarry has been found. There is no retreat.");}else Say(BossId=="astra"?"Astra rises. Her wrath will strike the entire party. There is no retreat.":Boss?"The guardian bars the descent. There is no retreat.":"Hostiles ahead. Movement is locked until combat ends.");
 }
 TurnDelay=0;PendingEnemy=-1;BuildOrder();AdvanceTurn();
}
void UDungeonModel::BuildOrder(){Order.Empty();for(int32 I=0;I<State->Party.Num();++I)if(State->Party[I].HP>0&&(!Betrayal||I==0)){FTurnActor A;A.Hero=true;A.Index=I;A.Speed=State->Party[I].Stats[1]*((State->Party[I].Status.Contains("Slow")||State->Party[I].Status.Contains("Chill"))?5:10)+Roll(5);Order.Add(A);}for(int32 I=0;I<Enemies.Num();++I)if(Enemies[I].HP>0){FTurnActor A;A.Hero=false;A.Index=I;A.Speed=Enemies[I].Speed*((Enemies[I].Status.Contains("Slow")||Enemies[I].Status.Contains("Chill"))?5:10)+Roll(5);Order.Add(A);}Order.StableSort([](const FTurnActor& A,const FTurnActor& B){return A.Speed>B.Speed;});Turn=0;}
void UDungeonModel::AdvanceTurn(){
 for(int32 Guard=0;Guard<100&&Combat;++Guard){
  RecordHardcoreDeaths();
  if(!Enemies.ContainsByPredicate([](const FEnemyUnit& E){return E.HP>0;})){FinishCombat();return;}
  if((State->Hardcore||Betrayal)&&State->Party[0].HP<=0){Defeat();return;}
  if(!State->Party.ContainsByPredicate([](const FPartyHero& H){return H.HP>0;})){Defeat();return;}
  if(Turn>=Order.Num()){Round++;BuildOrder();}if(Order.IsEmpty()){Defeat();return;}
  auto A=Order[Turn];int32& HP=A.Hero?State->Party[A.Index].HP:Enemies[A.Index].HP;if(HP<=0){Turn++;continue;}
  auto& Status=A.Hero?State->Party[A.Index].Status:Enemies[A.Index].Status;
  const bool Skip=DungeonStatusEffects::Tick(Status,HP,Floors[State->Floor].Rank);
  if(HP<=0||Skip){Turn++;continue;}
  if(A.Hero){Acting=A.Index;SelectedHero=Acting;return;}Acting=-1;if(TimedCombat){PendingEnemy=A.Index;TurnDelay=.65f;Say(Enemies[A.Index].Name+" prepares to attack...");return;}EnemyAction(A.Index);Turn++;
 }
}
void UDungeonModel::HitEnemy(int32 Index,int32 Damage,const FString& Type,const FString& Status){if(!Enemies.IsValidIndex(Index)||Enemies[Index].HP<=0)return;auto& E=Enemies[Index];float Mult=E.Resist.Contains(Type)?E.Resist[Type]:1.f;if(E.Status.Contains("Mark"))Mult*=1.3f;if(State->Meal>0)Mult*=1.1f;if(Betrayal)Mult*=3.5f;int32 Hit=FMath::Max(0,FMath::RoundToInt(Damage*Mult));E.HP=FMath::Max(0,E.HP-Hit);if(!Status.IsEmpty()&&Status!="Drain"&&Roll(100)<75)DungeonStatusEffects::Apply(E.Status,Status);Say(E.Name+FString::Printf(TEXT(" takes %d %s damage%s."),Hit,*Type,E.HP<=0?TEXT(" and falls"):TEXT("")));}
void UDungeonModel::EnemyAction(int32 Index){
 auto& E=Enemies[Index];TArray<int32> Living;for(int32 I=0;I<State->Party.Num();++I)if(State->Party[I].HP>0&&(!Betrayal||I==0))Living.Add(I);if(Living.IsEmpty())return;
 if(E.Id=="astra"){
  const FString Types[]={"Dark","Fire","Ice","Holy"};FString Type=Types[(Round-1)%4];CombatCues.Add(DungeonCombatAudio::ElementCue(Type));Say("Astra unleashes "+Type+" Cataclysm upon the entire party!");
  for(int32 I:Living){CombatFeedback(DungeonCombatAudio::ElementCue(Type),I,true,false);auto& H=State->Party[I];float Scale=E.HP<=E.MaxHP/2?1.3f:1.f;Scale*=DungeonStatusEffects::DamageScale(E.Status);int32 Damage=FMath::Max(1,FMath::RoundToInt(E.Attack*Scale)-H.Stats[4]/4);if(Type=="Holy"&&H.Class==5)Damage=FMath::Max(1,FMath::RoundToInt(Damage*.7f));if(H.Status.Contains("Guard"))Damage=FMath::Max(1,Damage/3);H.HP=FMath::Max(0,H.HP-Damage);Say(HeroName(H)+FString::Printf(TEXT(" takes %d %s damage%s."),Damage,*Type,H.HP<=0?TEXT(" and falls"):TEXT("")));}
  return;
 }
 int32 TargetHero=Living[Roll(Living.Num())];auto& H=State->Party[TargetHero];int32 Chance=FMath::Clamp(93-H.Stats[1]/10,78,97);if(E.Status.Contains("Blind"))Chance-=30;
 if(Roll(100)>=Chance){CombatFeedback("dodge",TargetHero,true);Say(HeroName(H)+" dodges "+E.Name+".");return;}
 FString Type=E.Type,Inflict=E.Inflict;float Scale=1.f;
 if(BossFight&&!Betrayal){int32 Signature=(Round-1)%3;Scale=Signature==0?1.f:Signature==1?1.15f:.85f;if(E.Id=="astra"){const FString Types[]={"Physical","Fire","Ice","Holy","Dark"};Type=Types[(Round-1)%5];Inflict=Type=="Fire"?"Burn":Type=="Ice"?"Stun":"Curse";}if(E.HP<=E.MaxHP/2)Scale*=1.2f;}
 Scale*=DungeonStatusEffects::DamageScale(E.Status);int32 Damage=FMath::Max(1,FMath::RoundToInt(E.Attack*Scale)-(Type=="Physical"?Armor(H):H.Stats[4]/4));if(Type=="Holy"&&H.Class==5)Damage=FMath::RoundToInt(Damage*.7f);if(H.Status.Contains("Guard"))Damage=FMath::Max(1,Damage/3);H.HP=FMath::Max(0,H.HP-Damage);
 if(!Inflict.IsEmpty()&&H.HP>0&&Roll(100)<FMath::Clamp(35-H.Stats[4]/3-H.Stats[5]/5,5,35))DungeonStatusEffects::Apply(H.Status,Inflict);
 CombatFeedback(H.Status.Contains("Guard")?FString("guard"):DungeonCombatAudio::ElementCue(Type),TargetHero,true);Say(E.Name+" strikes "+HeroName(H)+FString::Printf(TEXT(" for %d %s%s."),Damage,*Type,H.HP<=0?TEXT(". Fallen"):TEXT("")));
}
void UDungeonModel::Action(const FString& ActionName,int32 SkillIndex){
 if(TurnDelay>0||PendingEnemy>=0||!Combat||!State->Party.IsValidIndex(Acting)||State->Party[Acting].HP<=0)return;auto& H=State->Party[Acting];
 if(!Enemies.IsValidIndex(Target)||Enemies[Target].HP<=0){Target=Enemies.IndexOfByPredicate([](const FEnemyUnit& E){return E.HP>0;});}if(Target<0){FinishCombat();return;}
 if(ActionName=="Flee"){
  if(BossFight){Say("This battle cannot be escaped.");return;}
  int32 Dex=0,Count=0;for(auto Hero:State->Party)if(Hero.HP>0){Dex+=Hero.Stats[1];Count++;}int32 Chance=FMath::Clamp(60+Dex/FMath::Max(1,Count)-Enemies[Target].Speed,30,90);
  if(Roll(100)<Chance){CombatFeedback("escape",Acting,true);Combat=false;Acting=-1;State->X=ReturnX;State->Y=ReturnY;Enemies.Empty();for(auto& Hero:State->Party)Hero.Status.Empty();WorldDirty=true;Say("You retreat safely. The enemy remains in the passage.");return;}CombatFeedback("escape_fail",Acting,true);Say("Retreat failed.");
 }else if(ActionName=="Defend"){CombatFeedback("defend",Acting,true);H.Status.Add("Guard",2);Say(HeroName(H)+" braces for the next attack.");}
 else if(ActionName=="Item"){
  int32 Slot=H.Bag.IndexOfByPredicate([](const FBagItem& B){return B.Id=="health"||B.Id=="greater_health";});if(Slot<0){Say("No health potion in this hero's bag. Transfer supplies outside combat.");return;}if(!UseItem(Acting,Slot,Acting))return;CombatFeedback(ActionName=="Mana"?"mana":"item",Acting,true);
 }else if(ActionName=="Mana"){
  int32 Slot=H.Bag.IndexOfByPredicate([](const FBagItem& B){return B.Id=="mana"||B.Id=="greater_mana";});if(Slot<0){Say("No mana potion in this hero's bag.");return;}if(!UseItem(Acting,Slot,Acting))return;CombatFeedback(ActionName=="Mana"?"mana":"item",Acting,true);
 }else if(ActionName=="Attack"){
  int32 Accuracy=FMath::Clamp(93+H.Stats[1]/8+(H.Class==2?4:0)-(H.Status.Contains("Blind")?30:0),55,99);
  if(Roll(100)>=Accuracy){CombatFeedback("miss",Target);Say(HeroName(H)+" misses.");}else {float Crit=Roll(100)<FMath::Clamp(4+H.Stats[1]/6+H.Stats[5]/5+(H.Class==4?9:H.Class==2?4:0),4,40)?1.6f:1.f;if(Crit>1.f){CombatFeedback("critical",Target);Say("Critical strike!");}else CombatFeedback("hit",Target);HitEnemy(Target,FMath::RoundToInt(Attack(H)*Crit*DungeonStatusEffects::DamageScale(H.Status)),"Physical");}
 }else if(ActionName=="Skill"){
  if(H.Status.Contains("Silence")){Say("Silenced: attack, defend, or use health/mana potions until the effect fades.");return;}if(!Classes[H.Class].Skills.IsValidIndex(SkillIndex))return;auto D=Skill(Classes[H.Class].Skills[SkillIndex]);if(!D)return;
  if(H.Level<D->Unlock){Say(FString::Printf(TEXT("This skill unlocks at level %d."),D->Unlock));return;}if(H.MP<D->Cost){Say("Not enough MP. Use a mana potion or choose another action.");return;}
  if(D->Target=="revive"){
   if(State->Hardcore){Say("Resurrection cannot undo permanent death.");return;}int32 Dead=State->Party.IndexOfByPredicate([](const FPartyHero& Hero){return Hero.HP<=0;});if(Dead<0){Say("No ally needs resurrection.");return;}CombatFeedback(DungeonCombatAudio::SkillCue(*D),Dead,true,false);State->Party[Dead].HP=MaxHP(State->Party[Dead])/2;State->Party[Dead].Status.Empty();H.MP-=D->Cost;Say("A fallen companion returns to life.");
  }else if(D->Target=="heal"){
   int32 Recipient=Acting;float Lowest=2;for(int32 I=0;I<State->Party.Num();++I)if(State->Party[I].HP>0&&(!Betrayal||I==0)){float Ratio=float(State->Party[I].HP)/MaxHP(State->Party[I]);if(Ratio<Lowest){Lowest=Ratio;Recipient=I;}}
   CombatFeedback(DungeonCombatAudio::SkillCue(*D),Recipient,true,false);int32 Heal=FMath::RoundToInt((8+H.Stats[4]*2+H.Level*2)*D->Power*(H.Class==3?1.25f:1.f));auto& R=State->Party[Recipient];R.HP=FMath::Min(MaxHP(R),R.HP+DungeonStatusEffects::Healing(R.Status,Heal));H.MP-=D->Cost;Say(D->Name+" restores "+Classes[R.Class].Name+".");
  }else if(D->Target=="party"){H.MP-=D->Cost;for(int32 I=0;I<State->Party.Num();++I)if(!Betrayal||I==0){State->Party[I].Status.Add(D->Effect,3);CombatFeedback(DungeonCombatAudio::SkillCue(*D),I,true,false);}Say(D->Name+" protects the party.");}
  else {H.MP-=D->Cost;int32 Power=FMath::RoundToInt(SkillDamage(H,*D)*DungeonStatusEffects::DamageScale(H.Status));if(D->Target=="all"){for(int32 I=0;I<Enemies.Num();++I)if(Enemies[I].HP>0){CombatFeedback(DungeonCombatAudio::SkillCue(*D),I,false,false);HitEnemy(I,Power,D->Type,D->Effect);}}else {CombatFeedback(DungeonCombatAudio::SkillCue(*D),Target,false,false);HitEnemy(Target,Power,D->Type,D->Effect);}if(D->Effect=="Drain")H.HP=FMath::Min(MaxHP(H),H.HP+DungeonStatusEffects::Healing(H.Status,Power/2));}
  CombatCues.Add(DungeonCombatAudio::SkillCue(*D));
 }else return;
 Turn++;Acting=-1;if(TimedCombat){TurnDelay=.95f;}else AdvanceTurn();
}
void UDungeonModel::FinishCombat(){
 Combat=false;Acting=-1;TurnDelay=0;PendingEnemy=-1;int32 Rank=Floors[State->Floor].Rank;bool Astra=Enemies.ContainsByPredicate([](const FEnemyUnit& E){return E.Id=="astra";});
 if(Betrayal){State->Ending=2;State->Postgame=false;for(int32 I=1;I<State->Party.Num();++I)State->Party[I].HP=0;Screen="Ending";Say("Lonemoore burns. The portal bows to its new Hell Lord.");return;}
 for(auto& E:Enemies)HuntProgress(E.Family);int32 XP=(18+Rank*22)*Enemies.Num()*(BossFight?3:1);if(State->Floors[State->Floor].ContentVersion>=3&&!BossFight)XP=FMath::Max(1,FMath::RoundToInt(XP/10.f));State->Gold+=BossFight?500+State->Floor*125:(8+Rank*10)*Enemies.Num();GiveXP(XP);
 if(EncounterHunt>=0&&State->Hunts.IsValidIndex(EncounterHunt)){State->Hunts[EncounterHunt].Progress=State->Hunts[EncounterHunt].Goal;Say("The marked quarry falls. Return to the Hunt Board for your reward.");}
 else{State->Floors[State->Floor].Defeated.Add(EncounterCell,State->Transitions);if(BossFight){if(Tile(EncounterCell%Floors[State->Floor].Rows[0].Len(),EncounterCell/Floors[State->Floor].Rows[0].Len())=='B')State->Bosses.AddUnique(State->Floor);FBagItem Reward=EquipmentLoot(FMath::Clamp(2+State->Floor/6,2,4));if(!AddItem(Reward))State->PendingLoot.Add(Reward);Say(FString::Printf(TEXT("Guardian defeated: %d gold and quality equipment. Unclaimed rewards stay in your inventory ledger."),500+State->Floor*125));}else Say(FString::Printf(TEXT("Victory. %d XP for each recruited hero, and gold recovered."),XP));}
 AddItem(FBagItem(Roll(3)==0?"mana":"health"));if(Roll(5)==0)AddItem(EquipmentLoot(State->Postgame?3:Rank>9?2:1));
 if(State->Meal>0)State->Meal--;for(auto& H:State->Party)H.Status.Empty();
 if(Astra){State->Ending=1;State->Keys.AddUnique("Hell Key");Screen="Ending";Say("Astra falls. In Lonemoore, the first dawn breaks through the cloud.");Save("Victory");}
 else Save("Auto");WorldDirty=true;
}
void UDungeonModel::Defeat(){
 RecordHardcoreDeaths();
 Combat=false;Acting=-1;Enemies.Empty();WorldDirty=true;
 if(State->Hardcore||Betrayal){State->Dead=true;if(State->Hardcore&&!Testing)UGameplayStatics::SaveGameToSlot(State,DeathLedgerSlot("Fallen"),0);Screen="GameOver";Say(Betrayal?"Your companions stop your ascension. The pre-choice save remains available.":"Your journey has ended. Death is permanent in Hardcore.");return;}
 int32 Cost=ServicePrice("resurrect");if(State->Gold<Cost){Screen="GameOver";Say("The party falls. There is not enough gold for resurrection. Load an earlier save.");return;}
 State->Gold-=Cost;State->CorpseFloor=State->Floor;State->CorpseCell=Cell(State->X,State->Y);State->Corpse.Empty();
 for(auto& H:State->Party){State->Corpse.Append(H.Bag);H.Bag.Empty();for(auto& B:H.Gear)if(!B.Id.IsEmpty())State->Corpse.Add(B);H.Gear.Empty();H.Gear.SetNum(8);H.HP=MaxHP(H);H.MP=MaxMP(H);H.Status.Empty();}
 Town();Say("Resurrected in Lonemoore. One recovery bundle holds your belongings at the fall. Gold and keys are safe.");Save("Auto");
}
void UDungeonModel::RecordHardcoreDeaths(){
 if(!State->Hardcore)return;int32 Before=State->FallenClasses.Num();for(auto& H:State->Party)if(H.HP<=0)State->FallenClasses.AddUnique(H.Class);
 if(Before!=State->FallenClasses.Num()&&!Testing)UGameplayStatics::SaveGameToSlot(State,DeathLedgerSlot("Deaths"),0);
}

TArray<FString> UDungeonModel::EncounterRoster(int32 AtCell,bool Boss)const{
 const auto& D=Floors[State->Floor];int32 Hunt=HuntAt(AtCell);if(Hunt>=0)return {State->Hunts[Hunt].Monster};if(Boss)return {D.Boss};
 if(State->Floors[State->Floor].ContentVersion>=3)if(auto Source=State->Floors[State->Floor].EncounterSources.Find(AtCell))AtCell=*Source;
 FRandomStream Random(int32(GetTypeHash(State->RunId)^uint32(State->Floor*104729+AtCell*7919)));
 int32 Count=FMath::Clamp(1+D.Rank/5+Random.RandRange(0,State->Party.Num()/2),1,6);TArray<FString> Result;
 for(int32 I=0;I<Count;++I)Result.Add(D.Enemies[Random.RandRange(0,D.Enemies.Num()-1)]);return Result;
}
void UDungeonModel::TickCombat(float Delta){for(auto& Effect:CombatEffects)Effect.Age+=Delta;CombatEffects.RemoveAll([](const FCombatEffect& Effect){return Effect.Age>=DungeonCombatVFX::Duration(Effect.Cue);});if(!Combat||!TimedCombat||TurnDelay<=0)return;TurnDelay-=Delta;if(TurnDelay>0)return;TurnDelay=0;if(PendingEnemy>=0){int32 Index=PendingEnemy;PendingEnemy=-1;if(Enemies.IsValidIndex(Index)&&Enemies[Index].HP>0)EnemyAction(Index);Turn++;TurnDelay=.95f;RecordHardcoreDeaths();return;}AdvanceTurn();}
