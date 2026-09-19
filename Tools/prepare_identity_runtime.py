"""Stage a presentation-only runtime patch, applied after isolated UE validation."""
from pathlib import Path
G=Path(r'J:\First Person Dungeon Crawler Game');R=Path(r'J:\Lonemoore_Regional_Identity');D=R/'runtime';D.mkdir(exist_ok=True)
(D/'DungeonIdentityArt.h').write_text('''#pragma once
#include "CoreMinimal.h"
namespace IdentityArt {
// Cosmetic modules only: the original physics components remain authoritative.
FString MeshFor(int Region,int Floor,const FString& Module,float Height=0);
}
''')
(D/'DungeonIdentityArt.cpp').write_text('''#include "DungeonIdentityArt.h"
FString IdentityArt::MeshFor(int Region,int Floor,const FString& Module,float Height){
 if(Region<0||Region>8||Region==1)return FString();
 static const TCHAR* Codes[]={TEXT("Cathedral"),TEXT(""),TEXT("Catacombs"),TEXT("Warrens"),TEXT("Crypts"),TEXT("Fortress"),TEXT("Deep"),TEXT("Infernal"),TEXT("Hell")};
 if(Module!="Wall"&&Module!="Niche"&&Module!="Floor"&&Module!="Vault"&&Module!="Arch")return FString();
 FString Part=Module;
 if(Module=="Floor")Part=Height>500?"Cap":Region==8&&Floor==16?"ForgeFloor":Region==8&&Floor==17?"HaloFloor":"Floor";
 FString Name=FString("SM_ID_")+Codes[Region]+"_"+Part;
 return "/Game/RegionalIdentity/Meshes/"+Name+"."+Name;
}
''')
p=(G/'Source/DungeonCrawler/DungeonEnvironment.cpp').read_text()
assert '#include "DungeonIdentityArt.h"' not in p
p=p.replace('#include "DungeonRegionalArt.h"','#include "DungeonRegionalArt.h"\n#include "DungeonIdentityArt.h"')
needle='  }C->AddInstance(FTransform(Rot,P,Scale));'
assert p.count(needle)==1
p=p.replace(needle,needle+'''
  const FString VisualPath=IdentityArt::MeshFor(D.RegionIndex,S->Floor,Name,P.Z);
  if(!VisualPath.IsEmpty()){
   const FString VisualKey="ID_"+VisualPath;auto V=Batches.FindRef(VisualKey);
   if(!V){
    if(auto VisualMesh=LoadObject<UStaticMesh>(nullptr,*VisualPath)){
     V=NewObject<UInstancedStaticMeshComponent>(Root,*FString("ID_"+VisualMesh->GetName()));V->SetStaticMesh(VisualMesh);V->SetupAttachment(RC);
     if(Name=="Wall"||Name=="Niche")for(int I=0;I<V->GetNumMaterials();++I)if(auto Authored=V->GetMaterial(I)){
      const FString WorldPath=Authored->GetPathName().Replace(TEXT("MI_UV_"),TEXT("MI_"));
      if(auto WorldMaterial=LoadObject<UMaterialInterface>(nullptr,*WorldPath))V->SetMaterial(I,WorldMaterial);
     }
     V->SetCollisionEnabled(ECollisionEnabled::NoCollision);V->SetCastShadow(true);V->RegisterComponent();Batches.Add(VisualKey,V);
    }else UE_LOG(LogTemp,Error,TEXT("Missing regional identity mesh %s"),*VisualPath);
   }
   if(V){C->SetVisibility(false);C->SetCastShadow(false);V->AddInstance(FTransform(Rot,P,Scale));}
  }
''')
needle='Scenery.Add(A);DoorVisuals.Add({A,E.DoorId,P});'
assert p.count(needle)==1
p=p.replace(needle,needle+'''
   if(E.Kind=="Secret"){
    const FString VisualPath=IdentityArt::MeshFor(D.RegionIndex,S->Floor,"Wall");
    if(!VisualPath.IsEmpty())if(auto VisualMesh=LoadObject<UStaticMesh>(nullptr,*VisualPath)){
     auto V=NewObject<UStaticMeshComponent>(A,TEXT("ID_SecretVisual"));V->SetStaticMesh(VisualMesh);V->SetupAttachment(C);V->SetCollisionEnabled(ECollisionEnabled::NoCollision);V->RegisterComponent();
     C->SetVisibility(false,false);C->SetCastShadow(false);
    }
   }
''')
(D/'DungeonEnvironment.cpp').write_text(p)
p=(G/'Source/DungeonCrawler/DungeonRegionalArt.cpp').read_text()
line='  if(Wall%3==0){int Role=Sub>0&&Wall%2==0?7:1;Instance(Spec.Materials[Role],P+Inward*29+FVector(0,0,52),Rot,FVector(3.9,.018,1.04),true);}'
assert line in p;p=p.replace(line,'  // Regional wall geometry now supplies the facing; do not cover it with flat repair strips.')
(D/'DungeonRegionalArt.cpp').write_text(p)
p=(G/'Source/DungeonCrawler/DungeonRegionalArtReview.cpp').read_text()
p=p.replace('int W=D.Rows[0].Len(),Mismatch=0,Traced=0,Decor=0,Decals=0;', 'int W=D.Rows[0].Len(),Mismatch=0,Traced=0,Decor=0,Decals=0,IdentityMeshes=0;')
p=p.replace('for(auto C:Cs){if(C->GetName()', 'for(auto C:Cs){if(C->GetName().StartsWith("ID_")){++IdentityMeshes;Safe&=C->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&C->IsVisible();}if(C->GetName()')
p=p.replace('||Mat->GetPathName().Contains("/OldCitySewers/")','||Mat->GetPathName().Contains("/RegionalIdentity/")||Mat->GetPathName().Contains("/OldCitySewers/")')
needle='  bool KeyOK=!HoverKeys.IsEmpty();'
p=p.replace(needle,'  Check(D.RegionIndex==1?IdentityMeshes==0:IdentityMeshes>=5,FString::Printf(TEXT("Visible regional architecture batches: %d; collision-free"),IdentityMeshes));\n'+needle)
# Dedicated output folder keeps the previous review intact.
p=p.replace('TEXT("RegionalArt")','TEXT("RegionalIdentity")').replace('TEXT("RegionalArt/runtime_results.txt")','TEXT("RegionalIdentity/runtime_results.txt")')
(D/'DungeonRegionalArtReview.cpp').write_text(p)
print('RUNTIME_PATCH_STAGED',D)
