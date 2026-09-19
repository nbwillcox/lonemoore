#include "DungeonGame.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
#include "GameFramework/GameUserSettings.h"

void ADungeonController::TickIntegrityReview(float Delta){
 static double FrameTotal=0;static int FrameCount=0;static float Worst=0;static FString Report;static int Failures=0;static TArray<float> SettledFrames;
 if(ReviewTime>2&&Model->Screen=="Dungeon"&&!Model->WorldDirty)SettledFrames.Add(Delta*1000);
 FrameTotal+=Delta;FrameCount++;Worst=FMath::Max(Worst,Delta);ReviewTime+=Delta;if(ReviewTime<4)return;ReviewTime=0;
 auto M=Model.Get();auto S=M->State;auto Check=[&](bool OK,const FString& Name){Report+=(OK?"PASS ":"FAIL ")+Name+"\n";if(!OK)++Failures;UE_LOG(LogTemp,Display,TEXT("INTEGRITY_RUNTIME %s %s"),OK?TEXT("PASS"):TEXT("FAIL"),*Name);};
 auto Place=[&](int Floor,FString Room){M->EnterFloor(Floor);auto& R=S->Floors[Floor];int W=M->Floors[Floor].Rows[0].Len();int Chosen=-1;for(auto& Pair:R.Rooms)if(Pair.Value==Room){if(Chosen<0||Pair.Key<Chosen)Chosen=Pair.Key;}if(Chosen>=0){S->X=Chosen%W;S->Y=Chosen/W;}S->Facing=2;M->Reveal();M->WorldDirty=true;LastX=-1;};
 FString Evidence=FPaths::ProjectSavedDir()/TEXT("IntegrityUpdate")/(FParse::Param(FCommandLine::Get(),TEXT("IntegrityUltrawide"))?TEXT("Ultrawide"):TEXT("Standard"));
 auto Capture=[&](FString Name){FScreenshotRequest::RequestScreenshot(Evidence/(Name+TEXT(".png")),true,false);};
 switch(ReviewStep++){
 case 0:{M->NewGame(0,false);M->EnterFloor(0);auto Settings=UGameUserSettings::GetGameUserSettings();Settings->SetFullscreenMode(EWindowMode::Windowed);Settings->SetScreenResolution(FParse::Param(FCommandLine::Get(),TEXT("IntegrityUltrawide"))?FIntPoint(2560,1080):FIntPoint(1600,900));Settings->ApplySettings(false);break;}
 case 1:{
  int W=M->Floors[0].Rows[0].Len(),Closed=0,Open=0;auto& R=S->Floors[0];
  for(const auto& E:R.Boundaries){if(E.B<0)continue;FVector A(E.A%W*400,E.A/W*400,155),B(E.B%W*400,E.B/W*400,155);FHitResult Hit;bool Blocked=GetWorld()->LineTraceSingleByChannel(Hit,A,B,ECC_Visibility);bool ShouldBlock=!M->CanCross(0,E.A,E.B);if(Blocked!=ShouldBlock)Check(false,E.Id+" geometry disagrees: expected "+(ShouldBlock?"blocked":"clear")+" hit "+GetNameSafe(Hit.GetActor())+" / "+(Cast<UStaticMeshComponent>(Hit.GetComponent())?GetNameSafe(Cast<UStaticMeshComponent>(Hit.GetComponent())->GetStaticMesh()):GetNameSafe(Hit.GetComponent())));ShouldBlock?++Closed:++Open;}
  Check(Closed>0&&Open>0,FString::Printf(TEXT("Collision traced %d closed and %d open boundaries"),Closed,Open));
  int SweepMismatch=0,FloorMissing=0,CeilingMissing=0;
  for(const auto& B:R.Boundaries){FHitResult Hit;bool Blocked=GetWorld()->SweepSingleByChannel(Hit,FVector(B.A%W*400,B.A/W*400,155),FVector(B.B%W*400,B.B/W*400,155),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeCapsule(30,80));if(Blocked==M->CanCross(0,B.A,B.B))++SweepMismatch;}
  for(int Y=0;Y<M->Floors[0].Rows.Num();++Y)for(int X=0;X<W;++X)if(M->Tile(X,Y)!='#'){FHitResult Hit;FVector P(X*400,Y*400,155);if(!GetWorld()->LineTraceSingleByChannel(Hit,P,P-FVector(0,0,400),ECC_Visibility))++FloorMissing;if(!GetWorld()->LineTraceSingleByChannel(Hit,P,P+FVector(0,0,900),ECC_Visibility))++CeilingMissing;}
  Check(SweepMismatch==0,FString::Printf(TEXT("Player capsule sweeps: %d mismatches"),SweepMismatch));Check(FloorMissing==0&&CeilingMissing==0,FString::Printf(TEXT("Floor and ceiling seals: %d / %d missing"),FloorMissing,CeilingMissing));
  auto E=*R.Boundaries.FindByPredicate([](const FDungeonBoundary& E){return E.Mandatory;});S->X=E.A%W;S->Y=E.A/W;S->Facing=E.B%W>S->X?1:E.B%W<S->X?3:E.B/W>S->Y?2:0;M->Reveal();LastX=-1;
  Check(!M->Move(1),E.DoorId+" real movement rejects missing key");M->Interact();Check(!R.UnlockedDoors.Contains(E.DoorId),"Interaction cannot unlock missing key");break;}
 case 2:Capture("01_closed_gate_radar");break;
 case 3:{auto E=*S->Floors[0].Boundaries.FindByPredicate([](const FDungeonBoundary& E){return E.Mandatory;});S->Keys.AddUnique(M->Floors[0].Key);Check(!M->Move(1),"Correct key still closed");M->Interact();Check(!M->Move(1),"Door animation blocks movement");break;}
 case 4:{int W=M->Floors[0].Rows[0].Len();auto E=*S->Floors[0].Boundaries.FindByPredicate([](const FDungeonBoundary& E){return E.Mandatory;});FHitResult Hit;Check(!GetWorld()->LineTraceSingleByChannel(Hit,FVector(E.A%W*400,E.A/W*400,155),FVector(E.B%W*400,E.B/W*400,155),ECC_Visibility),"Opened physical doorway clear");S->Keys.Empty();Check(M->Move(1),"Movement through persistent opened gate after key removal");break;}
 case 5:Capture("02_open_gate");break;
 case 6:Place(4,"Ossuary");break;
 case 7:Capture("03_ossuary_gameplay");break;
 case 8:{M->Screen="Map";MapFloor=S->Floor;MapPan=FVector2D::ZeroVector;auto Before=FIntPoint(S->X,S->Y);Check(!M->Move(1)&&FIntPoint(S->X,S->Y)==Before,"Map blocks gameplay movement");M->State->Floors[S->Floor].Markers.AddUnique(M->Cell(S->X,S->Y));break;}
 case 9:{
  auto G=Interface->GetCachedGeometry();float Scale=FMath::Min(G.GetLocalSize().X/1600.f,G.GetLocalSize().Y/900.f);
  auto Pointer=[&](FVector2D P,FVector2D Last,FKey Key,float Wheel=0){auto ToAbs=[&](FVector2D V){return G.LocalToAbsolute((G.GetLocalSize()-FVector2D(1600,900)*Scale)/2+V*Scale);};TSet<FKey> Buttons;if(Key.IsValid())Buttons.Add(Key);return FPointerEvent(0,ToAbs(P),ToAbs(Last),Buttons,Key,Wheel,FModifierKeysState());};
  float Zoom=MapZoom;Interface->NativeOnMouseWheel(G,Pointer({625,455},{625,455},FKey(),1));Check(MapZoom>Zoom,"Mouse wheel zoom");
  Interface->NativeOnMouseButtonDown(G,Pointer({625,455},{625,455},EKeys::MiddleMouseButton));Interface->NativeOnMouseMove(G,Pointer({695,495},{625,455},EKeys::MiddleMouseButton));Interface->NativeOnMouseButtonUp(G,Pointer({695,495},{695,495},EKeys::MiddleMouseButton));Check(MapPan.Equals(FVector2D(70,40)),"Middle mouse pan");
  Check(ReviewClick("MapCenter")&&MapPan.IsNearlyZero(),"Recenter button");int C=M->Cell(S->X,S->Y);auto& R=S->Floors[S->Floor];R.Markers.Remove(C);Interface->NativeOnMouseButtonDown(G,Pointer({625,455},{625,455},EKeys::LeftMouseButton));Check(R.Markers.Contains(C),"Click adds marker on explored tile");
  Check(ReviewClick("MapFloor:-1")&&MapFloor==0,"Floor browsing skips unvisited floors");Check(ReviewClick("MapFloor:1")&&MapFloor==S->Floor,"Return to current visited floor");MapZoom=32;Capture("04_automap_fog");break;}
 case 10:Place(1,"Cistern");break;
 case 11:Capture("05_sewer_gameplay");break;
 case 12:Place(9,"Guard hall");break;
 case 13:Capture("06_fortress_gameplay");break;
 case 14:Place(14,"Ritual sanctuary");break;
 case 15:Capture("07_infernal_gameplay");break;
 default:{Report+=FString::Printf(TEXT("Frames %d, mean %.3f ms, worst %.3f ms (includes loading/rebuilds).\n"),FrameCount,1000*FrameTotal/FMath::Max(1,FrameCount),Worst*1000);SettledFrames.Sort();if(SettledFrames.Num())Report+=FString::Printf(TEXT("Settled gameplay frames %d: median %.3f ms, p95 %.3f ms. Samples begin two seconds after each review step; current machine and settings only.\n"),SettledFrames.Num(),SettledFrames[SettledFrames.Num()/2],SettledFrames[FMath::Min(SettledFrames.Num()-1,int(SettledFrames.Num()*.95))]);Report+=FString::Printf(TEXT("Runtime failures: %d\n"),Failures);FFileHelper::SaveStringToFile(Report,*(Evidence/TEXT("runtime_results.txt")));UE_LOG(LogTemp,Display,TEXT("INTEGRITY_REVIEW_COMPLETE failures=%d"),Failures);FPlatformMisc::RequestExit(false);break;}
 }
}

