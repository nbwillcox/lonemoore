"""Generate presentation-only region lookup data; hook activation follows isolated validation."""
from pathlib import Path
import json
R=Path(r'J:\Lonemoore_Regional_Art');G=Path(__file__).resolve().parents[1];regs=json.loads((R/'regions.json').read_text())
code='''#include "DungeonRegionalArt.h"
#include "DungeonGame.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
namespace {
struct FRegionalSpec {const TCHAR* Code;const TCHAR* Materials[8];const TCHAR* Props[3];const TCHAR* Decals[4];};
const FRegionalSpec Specs[]={
'''
for idx in range(9):
 r=next((v for v in regs if v['index']==idx),regs[0]);quoted=lambda xs:','.join('TEXT("'+x+'")' for x in xs)
 code+=' {TEXT("'+r['code']+'"),{'+quoted([a['id'] for a in r['materials']])+'},{'+quoted(r['props'])+'},{'+quoted(r['decals'])+'}},\n'
code+='''};
FString Material(const TCHAR* Id){return FString("/Game/RegionalArt/Materials/MI_")+Id+".MI_"+Id;}
}
FString RegionalArt::LeverFor(int Region){const FString Code=Specs[FMath::Clamp(Region,0,8)].Code;return "/Game/RegionalArt/Meshes/SM_"+Code+"_Lever.SM_"+Code+"_Lever";}
FString RegionalArt::KeyFor(const FString& Name){
 FString Type=Name.Contains("(Hell)")?"Hell":Name.Contains("Infernal")?"Infernal":Name.Contains("Drake")?"Drake":Name.Contains("Warden")?"Warden":Name.Contains("Crypt")?"Crypt":"Rusted";
 return "/Game/RegionalArt/Meshes/SM_Key_"+Type+".SM_Key_"+Type;
}
FString RegionalArt::MaterialFor(int Region,const FString& Name,const FString& Original,float Height){
 if(Region==1||Region<0||Region>8||Original=="I_Ember"||Original=="I_Soot")return Original;
 int Role=0;
 if(Original=="I_Rust"||Name=="Door"||Name=="Grate"||Name=="Brazier")Role=5;
 else if(Name=="Floor")Role=Height>500?3:2;
 else if(Name=="Vault")Role=3;
 else if(Name=="Arch"||Name=="Trim"||Name=="Pillar"||Name=="Stairs")Role=4;
 else if(Name=="Niche"||Name=="Tomb")Role=7;
 else if(Name=="Camp"||Name=="Mushrooms"||Original=="I_Bone")Role=6;
 return Material(Specs[Region].Materials[Role]);
}
void RegionalArt::Decorate(ADungeonController* Host,AActor* Root){
 auto M=Host->Model.Get();auto S=M->State;auto& D=M->Floors[S->Floor];int Region=D.RegionIndex;if(Region==1||Region<0||Region>8)return;
 auto& R=S->Floors[S->Floor];auto& Spec=Specs[Region];int W=D.Rows[0].Len(),Wall=0,Count=0,Decals=0;int Sub=D.Name.Contains("Level 3")?2:D.Name.Contains("Level 2")?1:0;
 TMap<FString,UInstancedStaticMeshComponent*> Batches;
 auto Instance=[&](FString Id,FVector P,FRotator Rot,FVector Scale,bool Panel=false){
  auto C=Batches.FindRef(Id);
  if(!C){FString Path=Panel?FString("/Engine/BasicShapes/Cube.Cube"):"/Game/RegionalArt/Meshes/SM_"+Id+".SM_"+Id;auto Mesh=LoadObject<UStaticMesh>(nullptr,*Path);if(!Mesh){UE_LOG(LogTemp,Error,TEXT("Regional mesh missing %s"),*Path);return;}
   C=NewObject<UInstancedStaticMeshComponent>(Root,*FString("REG_"+Id));C->SetStaticMesh(Mesh);C->SetupAttachment(Root->GetRootComponent());C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCastShadow(!Panel);
   if(Panel)C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,*Material(*Id)));
   C->RegisterComponent();Batches.Add(Id,C);}
  C->AddInstance(FTransform(Rot,P,Scale));++Count;
 };
 for(const auto& E:R.Boundaries){
  if(E.B<0||E.Kind!="Wall")continue;FVector A(E.A%W*400,E.A/W*400,0),B(E.B%W*400,E.B/W*400,0),P=(A+B)*.5f,Inward=(A-P).GetSafeNormal();FVector Tangent(-Inward.Y,Inward.X,0);++Wall;FRotator Rot(0,Inward.Rotation().Yaw-90,0);
  if(Wall%3==0){int Role=Sub>0&&Wall%2==0?7:1;Instance(Spec.Materials[Role],P+Inward*29+FVector(0,0,52),Rot,FVector(3.9,.018,1.04),true);}
  if(Wall%5==0){int Variant=(Wall/5+Sub)%3;Instance(Spec.Props[Variant],P+Inward*65+Tangent*88,Rot,FVector::OneVector);}
  if(Wall%4==0){int K=(Wall/4+Sub)%4;auto Mat=LoadObject<UMaterialInterface>(nullptr,*Material(Spec.Decals[K]));if(!Mat){UE_LOG(LogTemp,Error,TEXT("Missing regional decal %s"),Spec.Decals[K]);continue;}
   auto C=NewObject<UDecalComponent>(Root,*FString::Printf(TEXT("REG_Decal_%d"),Decals));C->SetupAttachment(Root->GetRootComponent());C->SetDecalMaterial(Mat);C->DecalSize=FVector(12,65,80);C->SetFadeScreenSize(.008f);C->RegisterComponent();C->SetWorldLocationAndRotation(P+Inward*32+FVector(0,0,90+K*22),Inward.Rotation());++Decals;}
 }
 UE_LOG(LogTemp,Display,TEXT("REGIONAL_ART floor=%d region=%d instances=%d decals=%d collision=unchanged"),S->Floor,Region,Count,Decals);
}
'''
(G/'Source/DungeonCrawler/DungeonRegionalArt.cpp').write_text(code)
print('Regional presentation helper prepared')
