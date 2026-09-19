#include "DungeonGame.h"
#include "DungeonCombatAudio.h"
#include "DungeonCombatVFX.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

// Uses the real action resolver and renderer with transient, save-isolated review state.
void ADungeonController::TickCombatFXReview(float Delta){
 static int32 Phase=0,Case=0,Failures=0;static FString Report,Label;
 const FString Folder=FPaths::ProjectSavedDir()/TEXT("CombatFXPass/Runtime");
 auto M=Model.Get();ReviewTime+=Delta;
 if(ReviewStep==0){
  ReviewStep=1;IFileManager::Get().MakeDirectory(*Folder,true);M->NewGame(0,false);M->Recruit(3);M->Recruit(6);M->EnterFloor(0);
  auto Settings=UGameUserSettings::GetGameUserSettings();Settings->SetScreenResolution(FIntPoint(1600,900));Settings->SetFullscreenMode(EWindowMode::Windowed);Settings->ApplySettings(false);
  MasterVolume=1.f;EffectsVolume=.75f;MusicVolume=.12f;ReviewTime=0;return;
 }
 if(Phase==0){
  if(ReviewTime<1.3f)return;
  if(Case>=31){FFileHelper::SaveStringToFile(Report,*(Folder/TEXT("runtime_results.txt")));UE_LOG(LogTemp,Display,TEXT("COMBAT_FX_REVIEW_COMPLETE cases=%d failures=%d"),Case,Failures);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);return;}
  M->Combat=true;M->Screen="Dungeon";M->TimedCombat=true;M->Acting=0;M->Target=1;M->Turn=0;M->PendingEnemy=-1;M->TurnDelay=0;M->BossFight=false;M->Betrayal=false;M->CombatEffects.Empty();M->CombatCues.Empty();
  for(auto& H:M->State->Party){H.Level=20;H.HP=100;H.MP=999;H.Status.Empty();}
  M->State->Party[0].Class=Case<21?Case/3:0;M->State->Party[0].HP=M->MaxHP(M->State->Party[0])-1;M->State->Party[0].Bag={FBagItem("health",5),FBagItem("mana",5)};
  const auto& D=M->EnemyDefs[0];FEnemyUnit E;E.Id=D.Id;E.Name="Effect review target";E.Art=D.Art;E.Type="Physical";E.HP=E.MaxHP=100000;E.Attack=1;E.Speed=1;M->Enemies.Init(E,3);
  if(Case<21){
   if(Case==11)M->State->Party[1].HP=0;
   if(Case==9||Case==17)M->State->Party[1].HP=1;
   Label=DungeonCombatAudio::SkillCue(*M->Skill(M->Classes[M->State->Party[0].Class].Skills[Case%3]));M->Action("Skill",Case%3);
  }else if(Case<=23){
   Label=Case==21?"hit":Case==22?"critical":"miss";
   for(int32 Try=0;Try<2048;++Try){M->Acting=0;M->TurnDelay=0;M->CombatCues.Empty();M->CombatEffects.Empty();M->Action("Attack");if(M->CombatCues.Contains(Label))break;}
  }else if(Case==24){Label="defend";M->Action("Defend");}
  else if(Case==25||Case==26){
   Label=Case==25?"dodge":"guard";for(auto& H:M->State->Party)H.Status.Add("Guard",2);
   for(int32 Try=0;Try<2048;++Try){M->CombatCues.Empty();M->CombatEffects.Empty();M->EnemyAction(0);if(M->CombatCues.Contains(Label))break;}
  }else if(Case==27){Label="item";M->Action("Item");}
  else if(Case==28){Label="mana";M->State->Party[0].MP=0;M->Action("Mana");}
  else if(Case==29){Label="lethal_hit";M->Enemies[1].HP=1;for(int32 Try=0;Try<2048;++Try){M->Acting=0;M->TurnDelay=0;M->CombatCues.Empty();M->CombatEffects.Empty();M->Action("Attack");if(M->Enemies[1].HP<=0)break;}}
  else {Label="boss_party_fire";M->Enemies[0].Id="astra";M->Round=2;M->EnemyAction(0);}
  const bool OK=M->CombatCues.Num()==1&&!M->CombatEffects.IsEmpty();if(!OK)++Failures;
  Report+=FString::Printf(TEXT("%s %s cues=%d sprites=%d\n"),OK?TEXT("PASS"):TEXT("FAIL"),*Label,M->CombatCues.Num(),M->CombatEffects.Num());
  for(const auto& FX:M->CombatEffects){const bool Loaded=CueSounds.Contains(FX.Cue);if(!Loaded)++Failures;UE_LOG(LogTemp,Display,TEXT("COMBAT_FX_EVENT case=%s cue=%s recipient=%d hero=%d loaded=%d"),*Label,*FX.Cue,FX.Target,FX.Hero?1:0,Loaded?1:0);}
  M->Acting=-1;M->TurnDelay=100.f;ReviewTime=0;Phase=1;return;
 }
 if(Phase==1&&ReviewTime>=.32f){FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("%02d_%s.png"),Case+1,*Label),true,false);Phase=2;return;}
 if(Phase==2&&ReviewTime>=1.3f){++Case;Phase=0;}
}
