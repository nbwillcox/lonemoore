#include "DungeonRegionalArt.h"
#include "DungeonGame.h"
#include "DungeonRenderUtils.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
namespace {
struct FRegionalSpec {const TCHAR* Code;const TCHAR* Materials[8];const TCHAR* Props[3];const TCHAR* Decals[4];};
const FRegionalSpec Specs[]={
 {TEXT("Cathedral"),{TEXT("Cathedral_PilgrimLimestone"),TEXT("Cathedral_RuinedLimePlaster"),TEXT("Cathedral_ProcessionalFlags"),TEXT("Cathedral_ChapelVault"),TEXT("Cathedral_BellCarving"),TEXT("Cathedral_OxidizedIron"),TEXT("Cathedral_OldPewOak"),TEXT("Cathedral_AltarDressing")},{TEXT("Cathedral_AltarFragment"),TEXT("Cathedral_BellAnchor"),TEXT("Cathedral_BrokenPew")},{TEXT("Cathedral_WaxRun"),TEXT("Cathedral_PlasterLoss"),TEXT("Cathedral_SootFan"),TEXT("Cathedral_DampFoot")}},
 {TEXT("Cathedral"),{TEXT("Cathedral_PilgrimLimestone"),TEXT("Cathedral_RuinedLimePlaster"),TEXT("Cathedral_ProcessionalFlags"),TEXT("Cathedral_ChapelVault"),TEXT("Cathedral_BellCarving"),TEXT("Cathedral_OxidizedIron"),TEXT("Cathedral_OldPewOak"),TEXT("Cathedral_AltarDressing")},{TEXT("Cathedral_AltarFragment"),TEXT("Cathedral_BellAnchor"),TEXT("Cathedral_BrokenPew")},{TEXT("Cathedral_WaxRun"),TEXT("Cathedral_PlasterLoss"),TEXT("Cathedral_SootFan"),TEXT("Cathedral_DampFoot")}},
 {TEXT("Catacombs"),{TEXT("Catacombs_BurialRubble"),TEXT("Catacombs_FuneraryPlaster"),TEXT("Catacombs_SettledFlags"),TEXT("Catacombs_BurialVault"),TEXT("Catacombs_CrownMemorial"),TEXT("Catacombs_AgedBronze"),TEXT("Catacombs_ChalkBone"),TEXT("Catacombs_NicheStone")},{TEXT("Catacombs_BoneShelf"),TEXT("Catacombs_MemorialPlaque"),TEXT("Catacombs_FuneraryUrn")},{TEXT("Catacombs_ChalkBloom"),TEXT("Catacombs_DustFall"),TEXT("Catacombs_WaxTrace"),TEXT("Catacombs_MemorialSpall")}},
 {TEXT("Warrens"),{TEXT("Warrens_PatchedRubble"),TEXT("Warrens_ScavengedPlanks"),TEXT("Warrens_TroddenEarth"),TEXT("Warrens_RootStrata"),TEXT("Warrens_TimberBracing"),TEXT("Warrens_BatteredIron"),TEXT("Warrens_RootMoss"),TEXT("Warrens_WovenScraps")},{TEXT("Warrens_SalvageBundle"),TEXT("Warrens_SealTray"),TEXT("Warrens_ThornCluster")},{TEXT("Warrens_MudSplash"),TEXT("Warrens_MossPatch"),TEXT("Warrens_SalvageScuff"),TEXT("Warrens_EarthFall")}},
 {TEXT("Crypts"),{TEXT("Crypts_SepulchralAshlar"),TEXT("Crypts_ScrapedMemorial"),TEXT("Crypts_VigilSlabs"),TEXT("Crypts_RibVaultStone"),TEXT("Crypts_FuneraryCarving"),TEXT("Crypts_TarnishedSilver"),TEXT("Crypts_CrimsonCloth"),TEXT("Crypts_CoffinStone")},{TEXT("Crypts_Nameplate"),TEXT("Crypts_CoffinCorner"),TEXT("Crypts_VotiveStand")},{TEXT("Crypts_ErasedName"),TEXT("Crypts_CandleWax"),TEXT("Crypts_CrimsonDust"),TEXT("Crypts_ColdSeep")}},
 {TEXT("Fortress"),{TEXT("Fortress_GarrisonBlocks"),TEXT("Fortress_DefenseMasonry"),TEXT("Fortress_GarrisonFlags"),TEXT("Fortress_UtilityVault"),TEXT("Fortress_MilitaryTrim"),TEXT("Fortress_WardenIron"),TEXT("Fortress_BarracksOak"),TEXT("Fortress_BarredPlate")},{TEXT("Fortress_ShackleFixture"),TEXT("Fortress_ShieldRack"),TEXT("Fortress_ChainGuide")},{TEXT("Fortress_RustRun"),TEXT("Fortress_BootScuff"),TEXT("Fortress_SootStain"),TEXT("Fortress_SaltSeep")}},
 {TEXT("Deep"),{TEXT("Deep_ChasmStrata"),TEXT("Deep_FracturedCave"),TEXT("Deep_CaveGravel"),TEXT("Deep_OverhangRock"),TEXT("Deep_MineralCrust"),TEXT("Deep_AncientIron"),TEXT("Deep_ScorchedDrakeRock"),TEXT("Deep_DarkSediment")},{TEXT("Deep_MineralCluster"),TEXT("Deep_DrakeScales"),TEXT("Deep_RockSpur")},{TEXT("Deep_MineralBloom"),TEXT("Deep_ClawScars"),TEXT("Deep_SootScorch"),TEXT("Deep_CaveSeep")}},
 {TEXT("Infernal"),{TEXT("Infernal_SealMasonry"),TEXT("Infernal_SealCarvedSlabs"),TEXT("Infernal_AshFlags"),TEXT("Infernal_FracturedVault"),TEXT("Infernal_RitualTrim"),TEXT("Infernal_HeatIron"),TEXT("Infernal_ScorchedPlaster"),TEXT("Infernal_HeatFissures")},{TEXT("Infernal_SealFragment"),TEXT("Infernal_RitualClamp"),TEXT("Infernal_OfferingVessel")},{TEXT("Infernal_AshFall"),TEXT("Infernal_BrokenSeal"),TEXT("Infernal_HeatScorch"),TEXT("Infernal_RitualResidue")}},
 {TEXT("Hell"),{TEXT("Hell_BasaltRampart"),TEXT("Hell_FracturedBasalt"),TEXT("Hell_ForgePlates"),TEXT("Hell_VaultBasalt"),TEXT("Hell_ObsidianCarving"),TEXT("Hell_InfernalMetal"),TEXT("Hell_ForgeSlag"),TEXT("Hell_EmberSeams")},{TEXT("Hell_SlagCluster"),TEXT("Hell_ChainAnchor"),TEXT("Hell_BrokenHalo")},{TEXT("Hell_AshDrift"),TEXT("Hell_ForgeScorch"),TEXT("Hell_BrokenSigil"),TEXT("Hell_SlagSpatter")}},
};
FString Material(const TCHAR* Id){return FString("/Game/RegionalArt/Materials/MI_")+Id+".MI_"+Id;}
}
FString RegionalArt::LeverFor(int Region){const FString Code=Specs[FMath::Clamp(Region,0,8)].Code;return "/Game/RegionalArt/Meshes/SM_"+Code+"_Lever.SM_"+Code+"_Lever";}
FString RegionalArt::FurnishingFor(int Region,int Variant){return FString(Specs[FMath::Clamp(Region,0,8)].Props[FMath::Abs(Variant)%3]);}
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
   C=DungeonBatch(Root,*FString("REG_"+Id));C->SetStaticMesh(Mesh);C->SetupAttachment(Root->GetRootComponent());C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCastShadow(!Panel);
   if(Panel)C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,*Material(*Id)));
   Batches.Add(Id,C);}
  C->AddInstance(FTransform(Rot,P,Scale));++Count;
 };
 for(const auto& E:R.Boundaries){
  if(E.B<0||E.Kind!="Wall")continue;FVector A(E.A%W*400,E.A/W*400,0),B(E.B%W*400,E.B/W*400,0),P=(A+B)*.5f,Inward=(A-P).GetSafeNormal();FVector Tangent(-Inward.Y,Inward.X,0);++Wall;FRotator Rot(0,Inward.Rotation().Yaw-90,0);
  // Regional wall geometry now supplies the facing; do not cover it with flat repair strips.
  if(Wall%5==0){int Variant=(Wall/5+Sub)%3;Instance(Spec.Props[Variant],P+Inward*65+Tangent*88,Rot,FVector::OneVector);}
  if(Wall%4==0){int K=(Wall/4+Sub)%4;auto Mat=LoadObject<UMaterialInterface>(nullptr,*Material(Spec.Decals[K]));if(!Mat){UE_LOG(LogTemp,Error,TEXT("Missing regional decal %s"),Spec.Decals[K]);continue;}
   auto C=NewObject<UDecalComponent>(Root,*FString::Printf(TEXT("REG_Decal_%d"),Decals));C->SetupAttachment(Root->GetRootComponent());C->SetDecalMaterial(Mat);C->DecalSize=FVector(12,65,80);C->SetFadeScreenSize(.008f);C->RegisterComponent();C->SetWorldLocationAndRotation(P+Inward*32+FVector(0,0,90+K*22),Inward.Rotation());++Decals;}
 }
 FinishDungeonBatches(Batches);
 UE_LOG(LogTemp,Display,TEXT("REGIONAL_ART floor=%d region=%d instances=%d decals=%d collision=unchanged"),S->Floor,Region,Count,Decals);
}
