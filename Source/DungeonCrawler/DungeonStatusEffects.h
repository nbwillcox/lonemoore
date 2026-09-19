#pragma once
#include "CoreMinimal.h"

// Save-compatible status keys. Durations count the afflicted actor's turns.
namespace DungeonStatusEffects {
inline int32 Duration(const FString& Key) { return Key=="Stun"||Key=="Fear" ? 1 : 3; }
inline void Apply(TMap<FString,int32>& Status,const FString& Key) {
 if(!Key.IsEmpty()&&Key!="Drain") Status.Add(Key,FMath::Max(Status.FindRef(Key),Duration(Key)));
}
inline float DamageScale(const TMap<FString,int32>& Status) { return (Status.Contains("Curse")?.7f:1.f)*(Status.Contains("Weakness")?.8f:1.f); }
inline int32 Healing(const TMap<FString,int32>& Status,int32 Amount) { return Status.Contains("Plague")?FMath::Max(1,Amount/2):Amount; }
inline void Cleanse(TMap<FString,int32>& Status) { const int32 Guard=Status.FindRef("Guard");Status.Empty();if(Guard>0)Status.Add("Guard",Guard); }
inline bool Tick(TMap<FString,int32>& Status,int32& HP,int32 Rank) {
 const bool Skip=Status.FindRef("Stun")>0||Status.FindRef("Fear")>0;
 for(auto& P:Status) { if(P.Value>0&&(P.Key=="Poison"||P.Key=="Bleed"||P.Key=="Burn"||P.Key=="Plague"))HP=FMath::Max(0,HP-(P.Key=="Plague"?FMath::Max(1,1+Rank/2):3+Rank)); --P.Value; }
 for(auto It=Status.CreateIterator();It;++It)if(It.Value()<=0)It.RemoveCurrent();
 return Skip;
}
inline FString EnemyStatus(const FString& Id,const FString& Existing) {
 if(Id=="fungal"||Id=="ratking")return "Plague";
 if(Id=="slime"||Id=="widow")return "Slow";
 if(Id=="rat"||Id=="succubus")return "Weakness";
 if(Id=="shaman"||Id=="gatewarden")return "Silence";
 if(Id=="brute"||Id=="minotaur"||Id=="gargoyle")return "Stun";
 if(Id=="archer"||Id=="hound"||Id=="bat")return "Bleed";
 return Existing;
}
}
