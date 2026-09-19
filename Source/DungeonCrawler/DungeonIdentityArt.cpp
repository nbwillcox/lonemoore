#include "DungeonIdentityArt.h"
FString IdentityArt::MeshFor(int Region,int Floor,const FString& Module,float Height){
 if(Region<0||Region>8||Region==1)return FString();
 static const TCHAR* Codes[]={TEXT("Cathedral"),TEXT(""),TEXT("Catacombs"),TEXT("Warrens"),TEXT("Crypts"),TEXT("Fortress"),TEXT("Deep"),TEXT("Infernal"),TEXT("Hell")};
 if(Module!="Wall"&&Module!="Niche"&&Module!="Floor"&&Module!="Vault"&&Module!="Arch")return FString();
 FString Part=Module;
 if(Module=="Floor")Part=Height>500?"Cap":Region==8&&Floor==16?"ForgeFloor":Region==8&&Floor==17?"HaloFloor":"Floor";
 FString Name=FString("SM_ID_")+Codes[Region]+"_"+Part;
 return "/Game/RegionalIdentity/Meshes/"+Name+"."+Name;
}
