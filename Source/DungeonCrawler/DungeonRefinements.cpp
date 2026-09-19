#include "DungeonModel.h"

bool UDungeonModel::CanDescend()const{const auto& D=Floors[State->Floor];const auto& R=State->Floors[State->Floor];bool Guardian=true;if(!D.Boss.IsEmpty())for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<D.Rows[Y].Len();++X)if(D.Rows[Y][X]=='B'&&!R.Defeated.Contains(Cell(X,Y)))Guardian=false;bool Gate=false;for(const auto& E:R.Boundaries)if(E.Mandatory){if(R.ContentVersion>=3&&!R.OpenDoors.Contains(E.DoorId))return false;if(R.OpenDoors.Contains(E.DoorId))Gate=true;}return Guardian&&Gate;}

int32 UDungeonModel::SkillDamage(const FPartyHero& H,const FSkillDef& D)const{return FMath::RoundToInt((D.Type=="Physical"?Attack(H):Magic(H)+Attack(H)*.5f)*D.Power*(H.Class==6&&D.Type=="Dark"?1.2f:1.f));}
FBagItem UDungeonModel::EquipmentLoot(int32 Quality){
 TArray<FString> Pool;for(const auto& D:Items){if(D.Slot.IsEmpty())continue;for(const auto& H:State->Party){const auto& C=Classes[H.Class];bool Allowed=D.Slot=="Weapon"?C.Weapons.Contains(D.Family):D.Slot=="Body"?C.Armor.Contains(D.Family):D.Slot=="Off Hand"?(H.Class==0||H.Class==5):true;if(Allowed){Pool.AddUnique(D.Id);break;}}}
 return FBagItem(Pool.IsEmpty()?"sword":Pool[Roll(Pool.Num())],1,Quality);
}
void UDungeonModel::RandomizeLayouts(){
 for(int F=0;F<Floors.Num();++F){FString Error;bool Valid=false;for(int Attempt=0;Attempt<3&&!Valid;++Attempt){GenerateModularFloor(F,int32(uint32(State->RandomSeed)+uint32(F*7919+Attempt*104729)));Valid=ValidateFloor(F,Error);}if(!Valid){GenerateModularFloor(F,73519);if(!ValidateFloor(F,Error)){UE_LOG(LogTemp,Fatal,TEXT("Validated modular floor fallback failed: %s"),*Error);}}}
}
