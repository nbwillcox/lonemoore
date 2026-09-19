#include "DungeonModel.h"
#include "DungeonStatusEffects.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"

namespace {
TArray<FString> Strings(const TSharedPtr<FJsonObject>& O,const TCHAR* K) { TArray<FString> R; for(auto V:O->GetArrayField(K))R.Add(V->AsString());return R; }
FString S(const TSharedPtr<FJsonObject>& O,const TCHAR* K){return O->GetStringField(K);}
int32 N(const TSharedPtr<FJsonObject>& O,const TCHAR* K){return int32(O->GetNumberField(K));}
const TArray<FString> GearSlots={"Weapon","Off Hand","Head","Body","Hands","Feet","Accessory 1","Accessory 2"};
const float Quality[]={.8f,1.f,1.12f,1.28f,1.5f};
}
bool UDungeonModel::Initialize() {
 FString Raw; TSharedPtr<FJsonObject> Root;
 if(!FFileHelper::LoadFileToString(Raw,*(FPaths::ProjectContentDir()/TEXT("Game/Data/campaign.json"))) || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Raw),Root))return false;
 Classes.Empty();Skills.Empty();Items.Empty();EnemyDefs.Empty();Floors.Empty();
 for(auto V:Root->GetArrayField(TEXT("classes"))){auto O=V->AsObject();FClassDef D;D.Name=S(O,TEXT("name"));D.Art=S(O,TEXT("art"));D.Passive=S(O,TEXT("passive"));D.Primary=N(O,TEXT("primary"));D.Skills=Strings(O,TEXT("skills"));D.Weapons=Strings(O,TEXT("weapons"));D.Armor=Strings(O,TEXT("armor"));for(auto A:O->GetArrayField(TEXT("stats")))D.Stats.Add(int32(A->AsNumber()));Classes.Add(D);}
 for(auto V:Root->GetArrayField(TEXT("skills"))){auto O=V->AsObject();FSkillDef D;D.Name=S(O,TEXT("name"));D.Type=S(O,TEXT("type"));D.Effect=S(O,TEXT("effect"));D.Target=S(O,TEXT("target"));D.Cost=N(O,TEXT("cost"));D.Unlock=N(O,TEXT("unlock"));D.Power=O->GetNumberField(TEXT("power"));Skills.Add(D);}
 for(auto V:Root->GetArrayField(TEXT("items"))){auto O=V->AsObject();FItemDef D;D.Id=S(O,TEXT("id"));D.Name=S(O,TEXT("name"));D.Family=S(O,TEXT("family"));D.Slot=S(O,TEXT("slot"));D.Effect=S(O,TEXT("effect"));D.Value=N(O,TEXT("value"));D.Power=N(O,TEXT("power"));D.Stack=N(O,TEXT("stack"));Items.Add(D);}
 for(auto V:Root->GetArrayField(TEXT("enemies"))){auto O=V->AsObject();FEnemyDef D;D.Id=S(O,TEXT("id"));D.Name=S(O,TEXT("name"));D.Art=S(O,TEXT("art"));D.Family=S(O,TEXT("family"));D.Type=S(O,TEXT("type"));D.Status=DungeonStatusEffects::EnemyStatus(D.Id,S(O,TEXT("status")));D.HP=O->GetNumberField(TEXT("hp"));D.Attack=O->GetNumberField(TEXT("attack"));D.Speed=O->GetNumberField(TEXT("speed"));for(auto P:O->GetObjectField(TEXT("resist"))->Values)D.Resist.Add(FString(P.Key),float(P.Value->AsNumber()));EnemyDefs.Add(D);}
 for(auto V:Root->GetArrayField(TEXT("floors"))){auto O=V->AsObject();FFloorDef D;D.Name=S(O,TEXT("name"));D.Region=S(O,TEXT("region"));D.Rows=Strings(O,TEXT("rows"));D.Enemies=Strings(O,TEXT("enemies"));D.Boss=S(O,TEXT("boss"));D.Key=S(O,TEXT("key"));D.Recruit=N(O,TEXT("recruit"));D.RecruitBark=S(O,TEXT("recruitBark"));D.Lore=S(O,TEXT("lore"));D.RegionIndex=N(O,TEXT("regionIndex"));D.Rank=N(O,TEXT("rank"));auto C=O->GetArrayField(TEXT("tint"));D.Tint=FLinearColor(C[0]->AsNumber(),C[1]->AsNumber(),C[2]->AsNumber());Floors.Add(D);}
 FString NamesRaw;TSharedPtr<FJsonObject> Names;
 if(!FFileHelper::LoadFileToString(NamesRaw,*(FPaths::ProjectContentDir()/TEXT("Game/Data/names.json")))||!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(NamesRaw),Names))return false;
 MaleNames=Strings(Names,TEXT("male"));FemaleNames=Strings(Names,TEXT("female"));
 BaseFloors=Floors;State=NewObject<UDungeonSave>(this);RefreshCharacters();return MaleNames.Num()==100&&FemaleNames.Num()==100&&Classes.Num()==7 && Floors.Num()>0;
}
void UDungeonModel::Say(const FString& Text){Notice=Text;Log.Add(Text);if(Log.Num()>500)Log.RemoveAt(0);if(State){State->History=Log;}UE_LOG(LogTemp,Display,TEXT("DUNGEON: %s"),*Text);}
int32 UDungeonModel::Roll(int32 Max){ State->RandomSeed=int32(uint32(State->RandomSeed)*1664525u+1013904223u);return Max>0 ? int32(uint32(State->RandomSeed)%uint32(Max)):0; }
const FItemDef* UDungeonModel::Item(const FString& Id)const{return Items.FindByPredicate([&](const FItemDef& I){return I.Id==Id;});}
const FSkillDef* UDungeonModel::Skill(const FString& Name)const{return Skills.FindByPredicate([&](const FSkillDef& I){return I.Name==Name;});}
int32 UDungeonModel::MaxHP(const FPartyHero& H)const{return 35+H.Stats[2]*6+H.Level*8;}
int32 UDungeonModel::MaxMP(const FPartyHero& H)const{return 8+(H.Stats[3]+H.Stats[4])*2+H.Level*3;}
int32 UDungeonModel::Armor(const FPartyHero& H)const{int32 A=H.Stats[2]/3+(H.Class==0?4:H.Class==5?3:0);for(int32 I=1;I<H.Gear.Num();++I)if(auto D=Item(H.Gear[I].Id))A+=ItemPower(H.Gear[I]);return A;}
int32 UDungeonModel::Attack(const FPartyHero& H)const{int32 A=4+H.Stats[0]+H.Level*2;if(H.Gear.Num()>0)if(auto D=Item(H.Gear[0].Id)){A+=ItemPower(H.Gear[0]);if(D->Family=="bow"||D->Family=="crossbow")A+=H.Stats[1]/2;if(D->Family=="magestaff"||D->Family=="holystaff")A+=H.Stats[3]/2;}return A;}
int32 UDungeonModel::Magic(const FPartyHero& H)const{return FMath::RoundToInt((5+H.Stats[3]*1.6f+H.Level*2)*(H.Class==1?1.2f:1.f));}
int32 UDungeonModel::NextXP(int32 Level)const{return Level*70+Level*Level*8;}
bool UDungeonModel::NewGame(int32 ClassIndex,bool Hardcore,const FString& CharacterName){
 if(!Classes.IsValidIndex(ClassIndex))return false;
 NameError.Empty();
 if(CharacterName.IsEmpty()){
  if(!Testing&&!AllowReviewNames){NameError=TEXT("Enter a name before beginning your journey.");return false;}
 }else{
  if(!ValidateCharacterName(CharacterName,NameError))return false;
  if(CharacterExists(CharacterName)){NameError=TEXT("That character already exists. Load their journey or choose another name.");return false;}
 }
 const auto PriorState=State;const auto PriorFloors=Floors;const auto PriorBoundaries=BoundaryIndices;const auto PriorLog=Log;
 const bool PriorCombat=Combat,PriorBetrayal=Betrayal,PriorDirty=WorldDirty;const int PriorHero=SelectedHero,PriorItem=SelectedItem,PriorEnemy=PendingEnemy;const float PriorDelay=TurnDelay;const FString PriorNotice=Notice;
 State=NewObject<UDungeonSave>(this);State->Hardcore=Hardcore;State->RunId=FGuid::NewGuid().ToString(EGuidFormats::Digits);State->CharacterName=CharacterName;State->Schema=CharacterName.IsEmpty()?2:3;
 State->Floors.SetNum(Floors.Num());Combat=false;Betrayal=false;Log.Empty();SelectedHero=0;SelectedItem=-1;
 State->RandomSeed=Testing?73519:int32(GetTypeHash(FGuid::NewGuid()));TurnDelay=0;PendingEnemy=-1;
 Floors=BaseFloors;BoundaryIndices.Empty();RandomizeLayouts();
 for(int32 Y=0;Y<Floors[0].Rows.Num();++Y)for(int32 X=0;X<Floors[0].Rows[Y].Len();++X)if(Floors[0].Rows[Y][X]=='S'){State->X=X;State->Y=Y;}
 State->CompanionNames.SetNum(7);TSet<FString> Used;if(!CharacterName.IsEmpty())Used.Add(CharacterName.ToLower());
 for(int32 I=0;I<7;++I){const auto& Names=(I==1||I==3||I==4)?FemaleNames:MaleNames;FString Name;do{Name=Names[Roll(Names.Num())];}while(Used.Contains(Name.ToLower()));Used.Add(Name.ToLower());State->CompanionNames[I]=Name;}
 Recruit(ClassIndex);if(!CharacterName.IsEmpty())State->Party[0].Name=CharacterName;
 AddItem(FBagItem("health",3));AddItem(FBagItem("mana",2));AddItem(FBagItem("food",2));GenerateOffers();
 if(!Testing&&!CharacterName.IsEmpty()){
  const FString Directory=CharacterDirectory(CharacterName);State->SavedUtc=FDateTime::UtcNow().ToIso8601();State->SavedLabel=TEXT("Autosave");
  const bool Existed=IFileManager::Get().DirectoryExists(*Directory);
  // Recheck after generation, before touching the folder. Existing journeys are never overwritten.
  const bool OwnReservation=!Directory.IsEmpty()&&!CharacterExists(CharacterName)&&IFileManager::Get().MakeDirectory(*Directory,true);
  const bool Stored=OwnReservation&&UGameplayStatics::SaveGameToSlot(State,CharacterSlot(CharacterName,TEXT("Auto")),0)&&WriteJourneyMetadata(TEXT("Auto"));
  if(!Stored){
   // These exact files can only have been created by this transaction in a previously empty folder.
   if(OwnReservation){for(const TCHAR* File:{TEXT("Auto.sav"),TEXT("Auto.json"),TEXT("Auto.json.tmp"),TEXT("profile.json"),TEXT("profile.json.tmp")})IFileManager::Get().Delete(*(Directory/File),false,true);if(!Existed)IFileManager::Get().DeleteDirectory(*Directory,false,false);}
   State=PriorState;Floors=PriorFloors;BoundaryIndices=PriorBoundaries;Log=PriorLog;Combat=PriorCombat;Betrayal=PriorBetrayal;WorldDirty=PriorDirty;SelectedHero=PriorHero;SelectedItem=PriorItem;PendingEnemy=PriorEnemy;TurnDelay=PriorDelay;Notice=PriorNotice;
   NameError=TEXT("Cannot create the character save. Your current journey is unchanged; check the name and disk space, then retry.");return false;
  }
 }
 PendingLoadCharacter.Empty();LastSavedSlot=!Testing&&!CharacterName.IsEmpty()?TEXT("Auto"):FString();SaveToast.Empty();Screen="Intro";IntroPanel=0;Say("The darkness pulled you to Lonemoore.");return true;
}
int32 UDungeonModel::RecruitForFloor(int32 Floor)const{
 if(!Floors.IsValidIndex(Floor))return -1;const int32 Authored=Floors[Floor].Recruit;
 if(!State||State->Party.IsEmpty()||Authored<0||Authored!=State->Party[0].Class)return Authored;
 for(int32 Class=0;Class<3;++Class)if(Class!=State->Party[0].Class)return Class;
 return -1;
}
void UDungeonModel::Recruit(int32 ClassIndex){
 if(!Classes.IsValidIndex(ClassIndex)||State->Party.Num()>=5||State->Recruited.Contains(ClassIndex)||State->Party.ContainsByPredicate([&](const FPartyHero& H){return H.Class==ClassIndex;}))return;
 FPartyHero H;H.Class=ClassIndex;H.Name=State->CompanionNames.IsValidIndex(ClassIndex)?State->CompanionNames[ClassIndex]:Classes[ClassIndex].Name;H.Stats=Classes[ClassIndex].Stats;H.Level=State->Party.Num()?State->Party[0].Level:1;H.XP=State->Party.Num()?State->Party[0].XP:0;
 for(int32 L=2;L<=H.Level;++L){H.Stats[Classes[ClassIndex].Primary]+=3;H.Stats[2]++;H.Stats[4]++;}
 H.Gear.SetNum(8);const FString Weapons[]={"sword","magestaff","bow","holystaff","dagger","broadsword","magestaff"};const FString Bodies[]={"leather","magerobes","leather","priestrobes","leather","chain","magerobes"};H.Gear[0]=FBagItem(Weapons[ClassIndex]);H.Gear[3]=FBagItem(Bodies[ClassIndex]);H.HP=MaxHP(H);H.MP=MaxMP(H);State->Party.Add(H);State->Recruited.Add(ClassIndex);WorldDirty=true;Say(Classes[ClassIndex].Name+" joins the journey at your level.");
}
void UDungeonModel::GiveXP(int32 Amount){for(auto& H:State->Party){if(State->Hardcore&&H.HP<=0)continue;H.XP+=Amount;while(H.Level<99&&H.XP>=NextXP(H.Level)){H.XP-=NextXP(H.Level);H.Level++;H.Points+=3;H.Stats[2]++;H.Stats[4]++;if(H.HP>0)H.HP=FMath::Min(MaxHP(H),H.HP+14);H.MP=FMath::Min(MaxMP(H),H.MP+5);Say(HeroName(H)+FString::Printf(TEXT(" reached level %d. Assign 3 attribute points [C]."),H.Level));}}}
void UDungeonModel::Allocate(int32 Hero,int32 Attribute){if(!State->Party.IsValidIndex(Hero)||Attribute<0||Attribute>5||Combat)return;auto& H=State->Party[Hero];if(H.Points>0&&H.HP>0){H.Points--;H.Stats[Attribute]++;}}
bool UDungeonModel::AddItem(FBagItem Incoming,int32 Preferred){
 auto D=Item(Incoming.Id);if(!D||Incoming.Count<1||Incoming.Quality<0||Incoming.Quality>4)return false;
 // Plan atomically so failed purchases and full bags cannot lose items or gold.
 auto Copy=State->Party;TArray<int32> Owners;if(Copy.IsValidIndex(Preferred))Owners.Add(Preferred);else for(int32 I=0;I<Copy.Num();++I)Owners.Add(I);
 for(int32 Owner:Owners){auto& H=Copy[Owner];if(State->Hardcore&&H.HP<=0)continue;for(auto& B:H.Bag)if(B.Id==Incoming.Id&&B.Quality==Incoming.Quality&&B.Count<D->Stack){int32 Add=FMath::Min(D->Stack-B.Count,Incoming.Count);B.Count+=Add;Incoming.Count-=Add;}while(Incoming.Count>0&&H.Bag.Num()<20){int32 Add=FMath::Min(D->Stack,Incoming.Count);H.Bag.Add(FBagItem(Incoming.Id,Add,Incoming.Quality));Incoming.Count-=Add;}if(Incoming.Count==0){State->Party=MoveTemp(Copy);return true;}}
 Say("Every bag is full. Make room before taking this item.");return false;
}
bool UDungeonModel::Transfer(int32 Owner,int32 Slot,int32 Recipient){if(Combat||Owner==Recipient||!State->Party.IsValidIndex(Owner)||!State->Party.IsValidIndex(Recipient)||!State->Party[Owner].Bag.IsValidIndex(Slot))return false;auto B=State->Party[Owner].Bag[Slot];if(AddItem(B,Recipient)){State->Party[Owner].Bag.RemoveAt(Slot);SelectedItem=-1;return true;}return false;}
bool UDungeonModel::UseItem(int32 Owner,int32 Slot,int32 Recipient){
 if(!State->Party.IsValidIndex(Owner)||!State->Party.IsValidIndex(Recipient)||!State->Party[Owner].Bag.IsValidIndex(Slot))return false;
 if(Combat&&Owner!=Acting)return false;auto& H=State->Party[Recipient];if(H.HP<=0)return false;auto& B=State->Party[Owner].Bag[Slot];auto D=Item(B.Id);if(!D||D->Effect.IsEmpty())return false;
 if(D->Effect=="hp")H.HP=FMath::Min(MaxHP(H),H.HP+DungeonStatusEffects::Healing(H.Status,D->Power));if(D->Effect=="mp")H.MP=FMath::Min(MaxMP(H),H.MP+D->Power);if(D->Effect=="cleanse")DungeonStatusEffects::Cleanse(H.Status);Say(HeroName(H)+" uses "+D->Name+".");if(--B.Count<=0)State->Party[Owner].Bag.RemoveAt(Slot);SelectedItem=-1;return true;
}
bool UDungeonModel::Equip(int32 Owner,int32 Slot,int32 AccessorySlot){
 if(Combat||!State->Party.IsValidIndex(Owner)||!State->Party[Owner].Bag.IsValidIndex(Slot))return false;auto& H=State->Party[Owner];if(H.HP<=0)return false;FBagItem B=H.Bag[Slot];auto D=Item(B.Id);if(!D||D->Slot.IsEmpty())return false;auto& C=Classes[H.Class];bool Allowed=D->Slot=="Weapon"?C.Weapons.Contains(D->Family):D->Slot=="Body"?C.Armor.Contains(D->Family):D->Slot=="Off Hand"?(H.Class==0||H.Class==5):true;if(!Allowed){Say("That equipment is not suitable for this class.");return false;}
 bool Accessory=D->Slot=="Accessory 1"||D->Slot=="Accessory 2";
 if(AccessorySlot<0||AccessorySlot>2||(!Accessory&&AccessorySlot!=0))return false;
 int32 G=Accessory&&AccessorySlot>0?5+AccessorySlot:GearSlots.Find(D->Slot);if(!H.Gear.IsValidIndex(G))return false;
 FBagItem Old=H.Gear[G];H.Gear[G]=B;if(Old.Id.IsEmpty())H.Bag.RemoveAt(Slot);else H.Bag[Slot]=Old;Say("Equipped "+D->Name+(Accessory?" in "+GearSlots[G]:FString())+".");SelectedItem=-1;return true;
}
bool UDungeonModel::Upgrade(int32 Owner,int32 GearSlot){if(Combat||!State->InTown||!State->Party.IsValidIndex(Owner)||!State->Party[Owner].Gear.IsValidIndex(GearSlot))return false;auto& B=State->Party[Owner].Gear[GearSlot];auto D=Item(B.Id);if(!D||B.Quality>=4)return false;int32 Cost=D->Value*(B.Quality+1)*2;if(State->Gold<Cost){Say("Not enough gold for that upgrade.");return false;}State->Gold-=Cost;B.Quality++;Say("The smith improves "+D->Name+".");Save("Auto");return true;}
int32 UDungeonModel::ServicePrice(const FString& Action)const{
 FString Key=Action;Key[0]=FChar::ToUpper(Key[0]);int32 Base=10,PerLevel=2;
 GConfig->GetInt(TEXT("DungeonBalance"),*(Key+"Base"),Base,GGameIni);GConfig->GetInt(TEXT("DungeonBalance"),*(Key+"PerLevel"),PerLevel,GGameIni);
 int32 Levels=0,Count=0;for(auto& H:State->Party){if(Action=="resurrect"&&H.HP>0)continue;Levels+=H.Level;Count++;}
 return Action=="resurrect"?Base*Count+PerLevel*Levels:Base+PerLevel*Levels;
}
void UDungeonModel::Buy(const FString& Id){auto D=Item(Id);if(!D||!State->InTown||Combat)return;if(State->Gold<D->Value){Say("Not enough gold.");return;}if(AddItem(FBagItem(Id))){State->Gold-=D->Value;Say("Purchased "+D->Name+".");Save("Auto");}}
void UDungeonModel::TownService(const FString& Action){
 if(!State->InTown||Combat)return;if(!NeedsCare(Action)){Say("No hero needs that care.");return;}if(Action=="resurrect"&&State->Hardcore){Say("Death is permanent in Hardcore.");return;}int32 Cost=ServicePrice(Action);if(State->Gold<Cost){Say("Not enough gold.");return;}
 if(Action=="gamble"){TArray<const FItemDef*> Gear;for(auto& D:Items)if(!D.Slot.IsEmpty())Gear.Add(&D);int32 R=Roll(100);if(AddItem(FBagItem(Gear[Roll(Gear.Num())]->Id,1,R<70?1:R<94?2:R<99?3:4))){State->Gold-=Cost;Say("The sealed purchase reveals its contents in your bag.");}return;}
 State->Gold-=Cost;for(auto& H:State->Party){if(Action=="resurrect"&&H.HP<=0){H.HP=MaxHP(H);H.MP=MaxMP(H);}if(H.HP<=0)continue;if(Action=="heal"||Action=="rest"){H.HP=MaxHP(H);H.Status.Empty();}if(Action=="rest")H.MP=MaxMP(H);}if(Action=="meal")State->Meal=3;Say(Action=="meal"?"A warm meal grants +10% damage for the next three fights.":"The party receives care.");Save("Auto");
}
void UDungeonModel::TickRecovery(float Delta){if(Combat||Screen!="Dungeon")return;float Interval=45;GConfig->GetFloat(TEXT("DungeonBalance"),TEXT("RegenerationSeconds"),Interval,GGameIni);RegenSeconds+=Delta;if(RegenSeconds>=FMath::Max(10.f,Interval)){RegenSeconds=0;for(auto& H:State->Party)if(H.HP>0){H.HP=FMath::Min(MaxHP(H),H.HP+1);H.MP=FMath::Min(MaxMP(H),H.MP+1);}}}
bool UDungeonModel::LoadStoredSlot(const FString& Path,const FString& ExpectedCharacter){
 if(Combat)return false;auto Loaded=Cast<UDungeonSave>(UGameplayStatics::LoadGameFromSlot(Path,0));
 if(!Loaded||(Loaded->Schema<1||Loaded->Schema>3)||Loaded->Party.IsEmpty()||Loaded->Party.Num()>5||!Floors.IsValidIndex(Loaded->Floor)||Loaded->Floors.Num()!=Floors.Num()){Say("This save is unavailable or incompatible.");return false;}
 FString NameValidation;
 if(Loaded->CharacterName!=ExpectedCharacter||(!ExpectedCharacter.IsEmpty()&&!ValidateCharacterName(ExpectedCharacter,NameValidation))){Say("This save does not belong to the selected character.");return false;}
 if(Loaded->Dead || (Loaded->Hardcore&&UGameplayStatics::DoesSaveGameExist(DeathLedgerSlot("Fallen",Loaded),0))){Say("This Hardcore journey has ended.");return false;}
 for(auto& H:Loaded->Party){if(!Classes.IsValidIndex(H.Class)||H.Stats.Num()!=6||H.Gear.Num()!=8||H.Bag.Num()>20||H.Level<1||H.Level>99){Say("Invalid hero data in save.");return false;}for(auto& B:H.Bag)if(!Item(B.Id)||B.Count<1||B.Count>Item(B.Id)->Stack||B.Quality<0||B.Quality>4)return false;for(auto& B:H.Gear)if(B.Quality<0||B.Quality>4||(!B.Id.IsEmpty()&&!Item(B.Id)))return false;}
 if(Loaded->Facing<0||Loaded->Facing>3)return false;
 // Validate the candidate and migrate in memory; never rewrite the source save on load.
 auto PriorState=State;auto PriorFloors=Floors;State=Loaded;Floors=BaseFloors;FString TopologyError;
 bool TopologyOK=RestoreTopology(TopologyError);const auto& Rows=Floors[Loaded->Floor].Rows;
 TopologyOK=TopologyOK&&Rows.IsValidIndex(Loaded->Y)&&Loaded->X>=0&&Loaded->X<Rows[Loaded->Y].Len()&&Rows[Loaded->Y][Loaded->X]!='#';
 State=PriorState;Floors=PriorFloors;for(int F=0;F<State->Floors.Num();++F)IndexBoundaries(F);if(!TopologyOK){Say("Save layout rejected: "+TopologyError);return false;}

 if(Loaded->Hardcore){auto Ledger=Cast<UDungeonSave>(UGameplayStatics::LoadGameFromSlot(DeathLedgerSlot("Deaths",Loaded),0));if(Ledger)for(int32 Class:Ledger->FallenClasses)Loaded->FallenClasses.AddUnique(Class);for(auto& H:Loaded->Party)if(Loaded->FallenClasses.Contains(H.Class))H.HP=0;if(Loaded->Party[0].HP<=0){Say("This Hardcore protagonist has permanently died.");return false;}} State=Loaded;Floors=BaseFloors;for(int32 F=0;F<Floors.Num();++F)if(!State->Floors[F].Layout.IsEmpty())Floors[F].Rows=State->Floors[F].Layout;for(int F=0;F<State->Floors.Num();++F)IndexBoundaries(F);Log=State->History;TurnDelay=0;PendingEnemy=-1;Combat=false;Betrayal=false;Enemies.Empty();Screen=State->Ending==1&&!State->Postgame?"Ending":State->InTown?"Town":"Dungeon";SelectedHero=0;SelectedItem=-1;WorldDirty=true;DoorOpening.Empty();Reveal();Say("Journey restored.");return true;
}

FString UDungeonModel::HeroName(const FPartyHero& H)const{return H.Name.IsEmpty()?Classes[H.Class].Name:H.Name;}
int32 UDungeonModel::ItemPower(const FBagItem& B)const{auto D=Item(B.Id);if(!D)return 0;if(D->Slot.IsEmpty())return D->Power;int32 Q=FMath::Clamp(B.Quality,0,4);return FMath::Max(Q+1,FMath::RoundToInt(D->Power*Quality[Q])+Q-1);}
FString UDungeonModel::ItemDetails(const FBagItem& B)const{auto D=Item(B.Id);if(!D)return "";if(D->Effect=="hp")return FString::Printf(TEXT("Restores %d HP"),D->Power);if(D->Effect=="mp")return FString::Printf(TEXT("Restores %d MP"),D->Power);if(D->Effect=="cleanse")return "Removes harmful conditions";if(D->Slot.IsEmpty())return "Trade relic / sell for gold";FString SlotLabel=D->Slot=="Accessory 1"||D->Slot=="Accessory 2"?FString("Accessory (either slot)"):D->Slot;return FString::Printf(TEXT("%s +%d / %s"),D->Slot=="Weapon"?TEXT("Attack"):TEXT("Armor"),ItemPower(B),*SlotLabel);}
int32 UDungeonModel::SellPrice(const FBagItem& B)const{auto D=Item(B.Id);return D?FMath::Max(1,FMath::RoundToInt(D->Value*Quality[FMath::Clamp(B.Quality,0,4)]*.5f)):0;}
const FBagItem* UDungeonModel::SaleItem(int32 Owner,int32 Slot)const{if(!State->Party.IsValidIndex(Owner))return nullptr;auto& H=State->Party[Owner];if(Slot>=100)return H.Gear.IsValidIndex(Slot-100)&&!H.Gear[Slot-100].Id.IsEmpty()?&H.Gear[Slot-100]:nullptr;return H.Bag.IsValidIndex(Slot)?&H.Bag[Slot]:nullptr;}
bool UDungeonModel::Sell(int32 Owner,int32 Slot){auto Found=SaleItem(Owner,Slot);if(Combat||!State->InTown||!Found)return false;auto& H=State->Party[Owner];if(State->Hardcore&&H.HP<=0)return false;auto B=*Found;int32 Price=SellPrice(B);if(Price<=0)return false;State->Gold+=Price;if(Slot>=100)H.Gear[Slot-100]=FBagItem();else if(--H.Bag[Slot].Count==0)H.Bag.RemoveAt(Slot);Say("Sold "+Item(B.Id)->Name+FString::Printf(TEXT(" for %d gold."),Price));Save("Auto");return true;}

bool UDungeonModel::NeedsCare(const FString& Action)const{if(Action=="resurrect")return !State->Hardcore&&State->Party.ContainsByPredicate([](const FPartyHero& H){return H.HP<=0;});if(Action=="heal"||Action=="rest")return State->Party.ContainsByPredicate([&](const FPartyHero& H){return H.HP>0&&(H.HP<MaxHP(H)||!H.Status.IsEmpty()||(Action=="rest"&&H.MP<MaxMP(H)));});return Action=="meal"||Action=="gamble";}
void UDungeonModel::CollectLoot(){while(!State->PendingLoot.IsEmpty()){if(!AddItem(State->PendingLoot[0]))return;State->PendingLoot.RemoveAt(0);}Say("All guardian rewards collected.");Save("Auto");}
