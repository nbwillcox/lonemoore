#include "DungeonGame.h"
#include "DungeonPresentation.h"
#include "DungeonCombatAudio.h"
#include "Sound/SoundBase.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

void ADungeonController::TickAdventurePolishReview(float Delta){
 static int Stage=0,Failures=0,Checkpoint=-1;static FString Report,FirstManual,SecondManual;
 if(FScreenshotRequest::IsScreenshotRequested())return;
 ReviewTime+=Delta;if(ReviewTime<2.f)return;ReviewTime=0;
 auto M=Model.Get();const FString Output=FPaths::ProjectSavedDir()/TEXT("AdventurePolish");
 auto Check=[&](bool OK,const FString& Text){Failures+=!OK;Report+=(OK?"PASS ":"FAIL ")+Text+"\n";UE_LOG(LogTemp,Display,TEXT("ADVENTURE_QA %s %s"),OK?TEXT("PASS"):TEXT("FAIL"),*Text);};
 auto Shot=[&](const FString& Name){FScreenshotRequest::RequestScreenshot(Output/(Name+".png"),true,false);};
 auto Key=[&](FKey K){Interface->NativeOnKeyDown(Interface->GetCachedGeometry(),FKeyEvent(K,FModifierKeysState(),0u,false,0,0));};
 auto Type=[&](const FString& Text){for(TCHAR C:Text)Interface->NativeOnKeyChar(Interface->GetCachedGeometry(),FCharacterEvent(C,FModifierKeysState(),0u,false));};
 auto Position=[&](int C,int Facing){int W=M->Floors[M->State->Floor].Rows[0].Len();M->State->X=C%W;M->State->Y=C/W;M->State->Facing=Facing;M->Reveal();LastX=-1;};
 auto SpriteChecks=[&](){
  int Enemies=0,Shrines=0;bool Feet=true,Centred=true,Metadata=true;
  for(auto A:Scenery)if(IsValid(A)){
   if(A->ActorHasTag(TEXT("DungeonEnemy"))){++Enemies;FString Art=A->Tags.Num()>1?A->Tags[1].ToString():FString();Metadata&=SpriteBottomPadding.Contains(Art);const float Height=A->GetActorScale3D().Y*100;const float Bottom=A->GetActorLocation().Z-Height*.5f+Height*SpriteBottomPadding.FindRef(Art);Feet&=FMath::IsNearlyEqual(Bottom,-DungeonPresentation::SpriteFloorInset,.1f);}
   if(A->ActorHasTag(TEXT("DungeonWaypoint"))){++Shrines;const FVector P=A->GetActorLocation();int X=FMath::RoundToInt(P.X/400),Y=FMath::RoundToInt(P.Y/400);Centred&=FMath::IsNearlyEqual(P.X,X*400.,.1)&&FMath::IsNearlyEqual(P.Y,Y*400.,.1)&&(M->Tile(X,Y)=='S'||M->Tile(X,Y)=='V');}
  }
  Check(Enemies>0&&Feet&&Metadata,FString::Printf(TEXT("%d enemy sprites have alpha-corrected feet at floor contact"),Enemies));
  Check(Shrines>=2&&Centred,FString::Printf(TEXT("%d shrine visuals share their exact map tile centres"),Shrines));
  Check(PlayerTorch.IsValid()&&PlayerTorchFill.IsValid()&&PlayerTorch->PointLightComponent->Intensity<32000&&FMath::IsNearlyEqual(PlayerTorchFill->PointLightComponent->Intensity,DungeonPresentation::PlayerFillIntensity,1.f),"Dimmed player torch and fill remain active");
 };
 switch(Stage++){
 case 0:{
  IFileManager::Get().MakeDirectory(*Output,true);M->Testing=false;M->AllowReviewNames=false;M->TimedCombat=false;
  M->SavePrefix="AdventureNative_"+FGuid::NewGuid().ToString(EGuidFormats::Digits)+"_";M->RefreshCharacters();
  Check(M->Characters.IsEmpty(),"Native review starts in an isolated profile root");
  bool Loaded=true;for(const auto& Id:DungeonCombatAudio::CueIds()){auto Sound=CueSounds.FindRef(Id);Loaded&=Sound&&Sound->GetDuration()>0.f;}
  Check(Loaded&&LoadedCues==DungeonCombatAudio::CueIds().Num(),FString::Printf(TEXT("All %d registered combat sounds are resident before play (loaded %d)"),DungeonCombatAudio::CueIds().Num(),LoadedCues));Command("New");break;
 }
 case 1:{bool All=true;for(int I=0;I<7;++I)All&=Interface->Hits.ContainsByPredicate([&](const FUIHit& H){return H.Id=="Hero:"+FString::FromInt(I);});Check(All,"All seven classes have selectable native controls");Shot("01_all_classes");break;}
 case 2:Check(ReviewClick("Hero:6"),"Warlock can be selected from new game");break;
 case 3:{Check(M->Screen=="NameHero"&&Interface->HasKeyboardFocus(),"Name entry has keyboard focus");Key(EKeys::Enter);Check(M->Screen=="NameHero"&&!M->NameError.IsEmpty(),"An empty name cannot begin a journey");Type("Morrox");Key(EKeys::BackSpace);Type("w");Check(M->NewHeroName=="Morrow","Native character input and backspace edit the name");Shot("02_name_character");break;}
 case 4:Key(EKeys::Enter);Check(M->Screen=="Intro"&&M->State->Party.Num()==1&&M->State->Party[0].Name=="Morrow"&&M->State->Party[0].Class==6,"Enter begins a named Warlock journey");break;
 case 5:Command("IntroNext");Command("IntroNext");Command("IntroNext");M->State->Gold=111;Check(M->Save("Quick"),"Morrow quicksave written");Command("Save");break;
 case 6:M->State->Gold=121;Check(ReviewClick("NewManual"),"Native new-save button works");FirstManual=M->LastSavedSlot;break;
 case 7:M->State->Gold=122;Command("NewManual");SecondManual=M->LastSavedSlot;Check(FirstManual!=SecondManual&&M->SaveSlots.Num()>=4,"Two manual saves coexist with auto and quick slots");Shot("03_multiple_saves");break;
 case 8:Command("BrowserBack");Command("Menu");Command("New");Command("Hero:4");Type("Selene");Key(EKeys::Enter);Check(M->Screen=="Intro"&&M->State->Party[0].Class==4,"Second named character starts as Rogue");break;
 case 9:Command("IntroNext");Command("IntroNext");Command("IntroNext");M->State->Gold=222;Check(M->Save("Quick"),"Selene has an independent quicksave");Command("Load");break;
 case 10:Check(M->Characters.Num()==2,"Load browser lists both named characters");Shot("04_character_browser");break;
 case 11:{int I=M->Characters.IndexOfByPredicate([](const FJourneySummary& R){return R.Character=="Morrow";});Check(I>=0&&ReviewClick("Character:"+FString::FromInt(I)),"Native character selector opens Morrow's saves");break;}
 case 12:{Shot("05_character_slots");break;}
 case 13:{int I=M->SaveSlots.IndexOfByPredicate([&](const FJourneySummary& R){return R.Slot==FirstManual;});Check(I>=0&&ReviewClick("SavedJourney:"+FString::FromInt(I)),"Native slot selector requests the older manual save");break;}
 case 14:Shot("06_load_confirmation");break;
 case 15:Check(ReviewClick("ConfirmLoad"),"Load confirmation is clickable");Check(M->State->CharacterName=="Morrow"&&M->State->Gold==121&&M->State->Party[0].Class==6,"Loading restores the chosen character and exact manual snapshot");M->EnterFloor(0);break;
 case 16:{SpriteChecks();Check(M->State->Floors[0].RoomPlacements.Num()==68,"New floor contains 68 sections, 34.6 percent fewer");const int W=M->Floors[0].Rows[0].Len(),C=M->Cell(M->State->X,M->State->Y);Position(C-W*2,2);break;}
 case 17:Shot("07_darker_arrival_and_shrine");break;
 case 18:{bool Found=false;const auto& Rows=M->Floors[0].Rows;const int W=Rows[0].Len(),DX[]={0,1,0,-1},DY[]={-1,0,1,0};for(int Y=2;Y<Rows.Num()-2&&!Found;++Y)for(int X=2;X<W-2&&!Found;++X)if(Rows[Y][X]=='E')for(int Dir=0;Dir<4&&!Found;++Dir){const int A=(Y-DY[Dir]*2)*W+X-DX[Dir]*2,B=(Y-DY[Dir])*W+X-DX[Dir],C=Y*W+X;if(Rows[A/W][A%W]=='.'&&M->CanCross(0,A,B)&&M->CanCross(0,B,C)){Position(A,Dir);Found=true;}}Check(Found,"An unobstructed normal enemy approach exists");break;}
 case 19:Shot("08_enemy_ground_contact");break;
 case 20:{
  auto& R=M->State->Floors[0];const auto* P=R.RoomPlacements.FindByPredicate([](const FRoomPlacement& V){return V.Id=="sealed_descent";});const int W=M->Floors[0].Rows[0].Len();if(!P){Check(false,"Descent chamber exists");break;}Checkpoint=(P->Y-1)*W+P->X-1;
  Position(Checkpoint,2);M->Interact();Check(!R.Shrines.Contains(Checkpoint),"Checkpoint rejects activation before the boss and key");
  M->State->Keys.AddUnique(M->Floors[0].Key);M->State->Bosses.AddUnique(0);
  for(int Y=0;Y<M->Floors[0].Rows.Num();++Y)for(int X=0;X<W;++X)if(M->Floors[0].Rows[Y][X]=='B')R.Defeated.Add(Y*W+X,M->State->Transitions);
  for(const auto& E:R.Boundaries)if(E.Mandatory)M->OpenBoundary(E);M->TickDoors(1.f);
  M->WorldDirty=true;Position(Checkpoint-W,2);break;
 }
 case 21:SpriteChecks();Shot("09_post_boss_save_shrine");break;
 case 22:Position(Checkpoint,2);M->Interact();Check(M->HasSave("Auto")&&M->State->SelectedWaypoint==Checkpoint&&M->Notice.Contains("saved"),"Descent shrine writes a real autosave and return point");Shot("10_checkpoint_saved");break;
 case 23:M->State->Gold=999;Check(M->Load("Auto")&&M->Cell(M->State->X,M->State->Y)==Checkpoint&&M->CanDescend()&&M->State->Gold==121,"Disk reload restores checkpoint position and completed gates");M->Town();break;
 case 24:Command("Enter");Shot("11_waypoint_selection");break;
 case 25:M->Waypoint(Checkpoint);Check(!M->State->InTown&&M->State->Floor==0&&M->Cell(M->State->X,M->State->Y)==Checkpoint,"Town teleport returns to the post-boss shrine");break;
 case 26:M->Screen="Map";MapFloor=0;Command("MapFit");Shot("12_smaller_floor_map");break;
 default:{
  const FString Root=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("SaveGames")/M->CharacterRoot());
  const FString Expected=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("SaveGames"))+TEXT("/");
  Check(M->SavePrefix.StartsWith("AdventureNative_")&&Root.StartsWith(Expected)&&IFileManager::Get().DeleteDirectory(*Root,false,true),"Only the uniquely named native-review profiles are removed");
  Report+=FString::Printf(TEXT("AdventurePolish native review failures=%d\n"),Failures);FFileHelper::SaveStringToFile(Report,*(Output/TEXT("native_results.txt")));
  UE_LOG(LogTemp,Display,TEXT("ADVENTURE_POLISH_REVIEW_COMPLETE failures=%d"),Failures);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);break;
 }
 }
}
