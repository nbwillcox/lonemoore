"""Promote validated regional content and activate presentation-only hooks after owner approval."""
from pathlib import Path
import json,shutil
G=Path(__file__).resolve().parents[1];R=Path(r'J:\Lonemoore_Regional_Art')
for f in ['unreal_textures.json','unreal_meshes.json']:assert json.loads((R/'reports'/f).read_text())['status']=='PASS',f
src=R/'UnrealTest/Content/RegionalArt';dst=G/'Content/RegionalArt';assert not dst.exists(),'Initial promotion refuses an existing content root'
for p in src.rglob('*.uasset'):
 if 'Showcase' in p.name:continue
 out=dst/p.relative_to(src);out.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,out)
p=G/'Source/DungeonCrawler/DungeonEnvironment.cpp';s=p.read_text();s=s.replace('#include "DungeonSewerArt.h"','#include "DungeonSewerArt.h"\n#include "DungeonRegionalArt.h"');s=s.replace('if(D.RegionIndex==1)Mat=SewerArt::MaterialFor(Name,Mat);','if(D.RegionIndex==1)Mat=SewerArt::MaterialFor(Name,Mat);else Mat=RegionalArt::MaterialFor(D.RegionIndex,Name,Mat,P.Z);')
s=s.replace('D.RegionIndex==1?SewerArt::MaterialFor(Name,DoorMat):"/Game/Game/Materials/M_"+DoorMat+".M_"+DoorMat','D.RegionIndex==1?SewerArt::MaterialFor(Name,DoorMat):RegionalArt::MaterialFor(D.RegionIndex,Name,DoorMat)')
s=s.replace(' SewerArt::Decorate(this,Root);',' SewerArt::Decorate(this,Root);RegionalArt::Decorate(this,Root);');p.write_text(s)
p=G/'Source/DungeonCrawler/DungeonGame.h';s=p.read_text();s=s.replace(' void TickSharedPropReview(float Delta);',' void TickSharedPropReview(float Delta);\n void TickRegionalArtReview(float Delta);\n void StartRegionalArtPlaytest();');p.write_text(s)
p=G/'Source/DungeonCrawler/DungeonGame.cpp';s=p.read_text();s=s.replace('#include "DungeonGame.h"','#include "DungeonGame.h"\n#include "DungeonRegionalArt.h"',1)
s=s.replace('ReviewMode=FParse::Param','ReviewMode=FParse::Param(FCommandLine::Get(),TEXT("RegionalArtReview"))||FParse::Param',1)
s=s.replace(' if(!BindingCapture.IsEmpty())',' if(M->Screen=="Menu"&&ReviewStep==0&&FParse::Param(FCommandLine::Get(),TEXT("RegionalArtPlaytest"))){ReviewStep=1;StartRegionalArtPlaytest();}\n if(!BindingCapture.IsEmpty())',1)
s=s.replace('void ADungeonController::TickReview(float Delta){','void ADungeonController::TickReview(float Delta){\n if(FParse::Param(FCommandLine::Get(),TEXT("RegionalArtReview"))){TickRegionalArtReview(Delta);return;}')
s=s.replace('*(MeshRoot+Name+".SM_"+Name)','*(Name=="SharedLever"?RegionalArt::LeverFor(D.RegionIndex):MeshRoot+Name+".SM_"+Name)')
old='if(T==\'K\'&&!R.Taken.Contains(C))Sprite("keys",P,65,D.Key.Contains("Rusted")?0:D.Key.Contains("Crypt")?1:D.Key.Contains("Warden")?2:D.Key.Contains("Drake")?4:3);'
new='''if(T=='K'&&!R.Taken.Contains(C)){
   auto SM=LoadObject<UStaticMesh>(nullptr,*RegionalArt::KeyFor(D.Key));if(SM){auto A=GetWorld()->SpawnActor<AStaticMeshActor>();A->SetMobility(EComponentMobility::Movable);A->SetActorEnableCollision(false);auto MC=A->GetStaticMeshComponent();MC->SetStaticMesh(SM);MC->SetCollisionEnabled(ECollisionEnabled::NoCollision);MC->SetCastShadow(false);FVector Origin=P+FVector(0,0,110);A->SetActorLocation(Origin);A->SetActorScale3D(FVector(1.4));Scenery.Add(A);HoverKeys.Add({A,Origin,float(P.X*.003+P.Y*.007)});}else UE_LOG(LogTemp,Error,TEXT("Missing regional key model %s"),*D.Key);
  }'''
assert old in s;s=s.replace(old,new)
s=s.replace(' for(auto B:Billboards)',' for(auto& K:HoverKeys)if(K.Actor.IsValid()){K.Actor->SetActorHiddenInGame(Model->Combat);auto To=DungeonCamera->GetActorLocation()-K.Actor->GetActorLocation();K.Actor->SetActorRotation(FRotator(0,To.Rotation().Yaw+90,0));}\n for(auto B:Billboards)',1);p.write_text(s)
# Maintain the existing dedicated shared-prop review after themed levers replace its source mesh.
p=G/'Source/DungeonCrawler/DungeonSharedPropReview.cpp';s=p.read_text();s=s.replace('C->GetStaticMesh()->GetPathName().Contains("/SharedInteractables/")','(C->GetStaticMesh()->GetPathName().Contains("/SharedInteractables/")||C->GetStaticMesh()->GetPathName().Contains("_Lever"))');p.write_text(s)
p=G/'Config/DefaultGame.ini';s=p.read_text();s=s.replace('+DirectoriesToAlwaysCook=(Path="/Game/SharedInteractables")','+DirectoriesToAlwaysCook=(Path="/Game/SharedInteractables")\n+DirectoriesToAlwaysCook=(Path="/Game/RegionalArt")');p.write_text(s)
d=G/'ArtReview/RegionalArt';d.mkdir(parents=True,exist_ok=True);shutil.copy2(R/'manifest.json',d/'manifest.json');(d/'SOURCE_LOCATION.txt').write_text(str(R))
print('REGIONAL_ART_PROMOTED',len(list(dst.rglob('*.uasset'))),'new assets')
