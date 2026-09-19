#include "DungeonGame.h"
#include "DungeonRoomKit.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "Engine/PointLight.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/FileManager.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "UnrealClient.h"

void ADungeonController::TickRoomKitReview(float Delta){
 static int Floor=0,Stage=-1,Failures=0,Sweeps=0,First=0,Last=17,BuildsAtEntry=0;
 static FString Report;static TSet<FString> CapturedKinds;
 static const bool Benchmark=FParse::Param(FCommandLine::Get(),TEXT("RoomKitBenchmark"));
 static FString BenchmarkScene=TEXT("Entrance");static double BenchmarkWarmUntil=0;
 enum class EAfterShot{None,Circular,Bridge,RoundedTurn,BossApproach,UnlockKey,FaceSeal,DefeatBoss};
 static EAfterShot AfterShot=EAfterShot::None;
 ReviewTime+=Delta;
 if(AfterShot!=EAfterShot::None){
  // Screenshot pixels and Slate UI are collected after PlayerTick. Keep the
  // model unchanged until that request has actually been consumed.
  if(FScreenshotRequest::IsScreenshotRequested())return;
 }else{if(ReviewTime<1.7f)return;ReviewTime=0;}
 auto M=Model.Get();
 auto Check=[&](bool OK,const FString& Message){Failures+=!OK;const FString Line=FString::Printf(TEXT("%s floor=%d %s\n"),OK?TEXT("PASS"):TEXT("FAIL"),Floor,*Message);Report+=Line;UE_LOG(LogTemp,Display,TEXT("ROOM_KIT_QA %s"),*Line);};
 if(Stage==-1){
  FParse::Value(FCommandLine::Get(),TEXT("ReviewFirst="),First);FParse::Value(FCommandLine::Get(),TEXT("ReviewLast="),Last);
  First=FMath::Clamp(First,0,17);Floor=First;Last=FMath::Clamp(Last,First,17);
  FParse::Value(FCommandLine::Get(),TEXT("RoomKitScene="),BenchmarkScene);
  IFileManager::Get().MakeDirectory(*(FPaths::ProjectSavedDir()/TEXT("RoomKit")),true);
  M->Testing=true;M->SavePrefix="RoomKitReview_"+FGuid::NewGuid().ToString(EGuidFormats::Digits)+"_";
  M->NewGame(0,false);M->State->RunId="RoomKitVisualReview73519";M->TimedCombat=false;Stage=0;return;
 }
 auto S=M->State;auto& D=M->Floors[Floor];auto& R=S->Floors[Floor];const int W=D.Rows[0].Len();
 auto Find=[&](TCHAR Tile)->int{for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<W;++X)if(D.Rows[Y][X]==Tile)return Y*W+X;return INDEX_NONE;};
 auto Position=[&](int C,int Facing){if(C<0||C>=W*D.Rows.Num())return;S->X=C%W;S->Y=C/W;S->Facing=Facing;M->Reveal();LastX=-1;};
 auto RoomPosition=[&](const FString& Id,FIntPoint Local,int Facing){
  const auto* P=R.RoomPlacements.FindByPredicate([&](const FRoomPlacement& Candidate){return Candidate.Id==Id;});
  if(!P){UE_LOG(LogTemp,Display,TEXT("ROOM_KIT_CAPTURE floor=%d has no %s module"),Floor,*Id);return false;}
  const auto Offset=DungeonRoomKit::RotateCell(Local,P->Rotation);int C=(P->Y+Offset.Y)*W+P->X+Offset.X;
  if(C<0||C>=W*D.Rows.Num()||D.Rows[C/W][C%W]=='#'||D.Rows[C/W][C%W]=='~')C=P->Y*W+P->X;
  Position(C,(Facing+P->Rotation)%4);CapturedKinds.Add(Id);return true;
 };
 auto Shot=[&](const FString& Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("RoomKit")/FString::Printf(TEXT("Floor_%02d_%s.png"),Floor+1,*Name),true,false);};
 if(FParse::Param(FCommandLine::Get(),TEXT("RoomKitDressingReview"))){
  struct FDetail {FString Name;FVector Position,Front;};
  static TArray<FDetail> Details;static int Detail=0,Capture=0;
  if(Stage==0){M->EnterFloor(Floor);Details.Empty();Detail=0;Stage=1;return;}
  if(Stage==1){
   for(auto Actor:Scenery)if(IsValid(Actor)){TArray<UInstancedStaticMeshComponent*> Components;Actor->GetComponents(Components);
    for(auto C:Components)if(auto Mesh=C->GetStaticMesh())if(Mesh->GetPathName().Contains(TEXT("/RegionalDressing/Meshes/"))&&C->GetInstanceCount()>0){
     FTransform T;C->GetInstanceTransform(0,T,true);Details.Add({Mesh->GetName(),T.TransformPosition(Mesh->GetBounds().Origin),T.TransformVectorNoScale(FVector(0,1,0))});
     Check(C->GetCollisionEnabled()==ECollisionEnabled::NoCollision,"Regional scenery preserves gameplay collision: "+Mesh->GetName());
    }
   }
   Check(!Details.IsEmpty(),"Regional prop batches are visible in the native world");Stage=2;
  }
  if(FScreenshotRequest::IsScreenshotRequested())return;
  if(Detail>=Details.Num()){
   if(++Floor<=Last){Stage=0;Capture=0;return;}
   UE_LOG(LogTemp,Display,TEXT("ROOM_KIT_DRESSING_REVIEW_COMPLETE floors=%d failures=%d"),Last-First+1,Failures);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);return;
  }
  const auto& V=Details[Detail];
  if(!Capture){
   int Best=-1,Facing=0;float Score=1.e20f;
   const int CX=FMath::RoundToInt(V.Position.X/400),CY=FMath::RoundToInt(V.Position.Y/400);
   for(int Y=CY-4;Y<=CY+4;++Y)for(int X=CX-4;X<=CX+4;++X){if(Y<0||Y>=D.Rows.Num()||X<0||X>=W||D.Rows[Y][X]!='.')continue;
    const FVector Eye(X*400,Y*400,155),DeltaTo=V.Position-Eye;const float Distance=DeltaTo.Size2D();if(Distance<260||Distance>1450||FVector::DotProduct((-DeltaTo).GetSafeNormal2D(),V.Front)<.2f)continue;
    const float Yaw=DeltaTo.Rotation().Yaw;const int CandidateFacing=(FMath::RoundToInt(Yaw/90)+5)%4;const float Angle=FMath::Abs(FMath::FindDeltaAngleDegrees(Yaw,(CandidateFacing-1)*90.f));if(Angle>30)continue;
    FHitResult Hit;if(GetWorld()->LineTraceSingleByChannel(Hit,Eye,V.Position,ECC_Visibility))continue;
    const float CandidateScore=FMath::Abs(Distance-600.f)+Angle*8;if(CandidateScore<Score){Best=Y*W+X;Facing=CandidateFacing;Score=CandidateScore;}
   }
   Check(Best>=0,"Accessible front view of "+V.Name);if(Best<0){++Detail;return;}Position(Best,Facing);Capture=1;return;
  }
  if(Capture==1){
   // Aim the review camera at the actual prop, then allow temporal rendering
   // to settle before capture. Gameplay camera behavior remains unchanged.
   RotationTo=(V.Position-DungeonCamera->GetActorLocation()).Rotation();
   RotationTo.Pitch=FMath::Clamp(RotationTo.Pitch,-18.f,5.f);RotationFrom=RotationTo;
   DungeonCamera->SetActorRotation(RotationTo);Capture=2;return;
  }
  FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("RoomKit")/FString::Printf(TEXT("Floor_%02d_Prop_%s.png"),Floor+1,*V.Name),false,false);
  ++Detail;Capture=0;return;
 }
 if(FParse::Param(FCommandLine::Get(),TEXT("RoomKitFloorDiagnostic"))){
  // Exercise the views missing from the original review: all four directions
  // while standing inside each shell, including its near floor at ultrawide FOV.
  static TArray<FString> Rooms;static int Sample=0;static bool CaptureNext=false;
  if(Stage==0){M->EnterFloor(Floor);Stage=1;return;}
  if(Stage==1){
   FString Only;FParse::Value(FCommandLine::Get(),TEXT("DiagnosticRoom="),Only);
   for(const auto& P:R.RoomPlacements)if(Only.IsEmpty()||P.Id==Only)Rooms.AddUnique(P.Id);
   Check(!Rooms.IsEmpty(),"Floor diagnostic has matching room placements");Stage=2;
  }
  if(FScreenshotRequest::IsScreenshotRequested())return;
  if(Sample>=Rooms.Num()*4){UE_LOG(LogTemp,Display,TEXT("ROOM_KIT_FLOOR_DIAGNOSTIC_COMPLETE floor=%d rooms=%d views=%d failures=%d"),Floor,Rooms.Num(),Sample,Failures);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);return;}
  if(!CaptureNext){RoomPosition(Rooms[Sample/4],FIntPoint(0,0),Sample%4);CaptureNext=true;return;}
  Shot(FString::Printf(TEXT("FloorCheck_%s_%d"),*Rooms[Sample/4],Sample%4));++Sample;CaptureNext=false;return;
 }
 if(AfterShot!=EAfterShot::None){
  const EAfterShot Action=AfterShot;AfterShot=EAfterShot::None;
  switch(Action){
   case EAfterShot::Circular:RoomPosition("circular_ossuary",FIntPoint(0,-2),2);break;
   case EAfterShot::Bridge:if(!RoomPosition("void_bridge",FIntPoint(0,-2),2))RoomPosition("fire_bridge",FIntPoint(0,-2),2);break;
   case EAfterShot::RoundedTurn:RoomPosition("rounded_turn",FIntPoint(0,-2),2);break;
   case EAfterShot::BossApproach:Check(RoomPosition("boss_approach",FIntPoint(0,-2),2),"Dedicated ominous boss approach exists");break;
   case EAfterShot::UnlockKey:{
    int Exit=Find('>');if(Exit<0)Exit=Find('A');Position(Exit,0);M->Interact();Check(S->Floor==Floor&&M->Screen=="Dungeon"&&!M->CanDescend(),"Forced stair position cannot bypass locked approach");
    S->Keys.AddUnique(D.Key);for(const auto& E:R.Boundaries)if(E.Mandatory&&E.Requirement!="guardian")M->OpenBoundary(E);M->TickDoors(1);M->WorldDirty=true;RoomPosition("boss_arena",FIntPoint(0,-2),2);break;
   }
   case EAfterShot::FaceSeal:RoomPosition("boss_arena",FIntPoint(0,2),2);break;
   case EAfterShot::DefeatBoss:{
    const int Boss=Find('B');if(Boss>=0){R.Defeated.Add(Boss,S->Transitions);S->Bosses.AddUnique(Floor);}
    for(const auto& E:R.Boundaries)if(E.Mandatory)M->OpenBoundary(E);M->TickDoors(1);M->WorldDirty=true;break;
   }
   default:break;
  }
  // Each next scene receives the complete settling interval after the move.
  ReviewTime=0;return;
 }
 if(Benchmark){
  if(Stage==0){M->EnterFloor(Floor);Stage=1;return;}
  if(Stage==1){
   bool Positioned=true;
   if(BenchmarkScene.Equals(TEXT("BossApproach"),ESearchCase::IgnoreCase))Positioned=RoomPosition("boss_approach",FIntPoint(0,-2),2);
   else if(BenchmarkScene.Equals(TEXT("Bridge"),ESearchCase::IgnoreCase))Positioned=RoomPosition("void_bridge",FIntPoint(0,-2),2)||RoomPosition("fire_bridge",FIntPoint(0,-2),2);
   else if(!BenchmarkScene.Equals(TEXT("Entrance"),ESearchCase::IgnoreCase))Positioned=false;
   if(!Positioned){UE_LOG(LogTemp,Error,TEXT("ROOM_KIT_BENCHMARK_FAILED scene=%s floor=%d"),*BenchmarkScene,Floor);FPlatformMisc::RequestExitWithStatus(false,1);return;}
   // Startup and shader warm-up are excluded from the measured capture.
   BenchmarkWarmUntil=FPlatformTime::Seconds()+8;Stage=2;return;
  }
  if(Stage==2&&FPlatformTime::Seconds()>=BenchmarkWarmUntil){
#if CSV_PROFILER
   FCsvProfiler::Get()->BeginCapture(2400);
   UE_LOG(LogTemp,Display,TEXT("ROOM_KIT_BENCHMARK_READY floor=%d scene=%s rooms=%d frames=2400"),Floor,*BenchmarkScene,R.RoomPlacements.Num());Stage=3;
#else
   UE_LOG(LogTemp,Error,TEXT("ROOM_KIT_BENCHMARK_FAILED CSV profiler unavailable"));FPlatformMisc::RequestExitWithStatus(false,1);
#endif
  }
  return;
 }
 auto Sweep=[&](bool GatesOnly){
  int Count=0,Mismatches=0;TMap<FString,int> Classes;
  for(const auto& E:R.Boundaries){
   if(GatesOnly&&!E.Mandatory)continue;
   if(E.A<0||E.B<0||E.A>=W*D.Rows.Num()||E.B>=W*D.Rows.Num())continue;
   TCHAR A=D.Rows[E.A/W][E.A%W],B=D.Rows[E.B/W][E.B%W];if((A=='~'||A=='#')&&(B=='~'||B=='#'))continue;
   int From=E.A,To=E.B;if(A=='~'||A=='#')Swap(From,To);
   FHitResult Hit;const bool Block=GetWorld()->SweepSingleByChannel(Hit,FVector(From%W*400,From/W*400,155),FVector(To%W*400,To/W*400,155),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeCapsule(30,80));
   if(Block==M->CanCross(Floor,From,To)){
    if(Mismatches<16){const FString Detail=FString::Printf(TEXT("Boundary %s %d->%d kind=%s expected_block=%d actual_block=%d hit=%s/%s\n"),*E.Id,From,To,*E.Kind,!M->CanCross(Floor,From,To),Block,*GetNameSafe(Hit.GetActor()),*GetNameSafe(Hit.GetComponent()));Report+=Detail;UE_LOG(LogTemp,Display,TEXT("ROOM_KIT_SWEEP %s"),*Detail);}++Mismatches;
   }
   ++Count;++Classes.FindOrAdd(E.Kind);
  }
  Sweeps+=Count;Check(Count>0&&Mismatches==0,FString::Printf(TEXT("%s physical sweeps=%d mismatches=%d"),GatesOnly?TEXT("Gate"):TEXT("All boundary"),Count,Mismatches));
  if(!GatesOnly)for(const auto& Pair:Classes)Report+=FString::Printf(TEXT("floor=%d sweep_class=%s count=%d\n"),Floor,*Pair.Key,Pair.Value);
 };
 switch(Stage++){
 case 0:
  M->Combat=false;M->Enemies.Empty();S->Keys.Empty();S->Bosses.Empty();M->EnterFloor(Floor);break;
 case 1:{
  FString Error;Check(R.ContentVersion==4&&M->ValidateFloor(Floor,Error),"Authored floor prerequisites "+Error);BuildsAtEntry=ArchitectureBuilds;Sweep(false);
  const auto* Entrance=R.RoomPlacements.FindByPredicate([](const FRoomPlacement& P){return P.Id=="arrival_chamber";});
  Check(Entrance&&(Entrance->ActiveSockets&(1<<S->Facing)),"Arrival faces a connected passage");
  Check(PlayerTorch.IsValid()&&PlayerTorchFill.IsValid(),"Player torch and ambient fill present");Check(R.MemoryTiles.Num()<R.OriginalArea,"Entrance fog hides the unexplored floor");
  TMap<FString,int> Counts;TMap<FString,TArray<FTransform>> Transforms;int Stairs=0,OldStairs=0,Modules=0;bool Materials=true,UnitScale=true,SurfaceSections=true;const FString Stair=FString::Printf(TEXT("SM_Stair_%02d"),Floor);
  for(auto Actor:Scenery){if(!IsValid(Actor))continue;TArray<UInstancedStaticMeshComponent*> Components;Actor->GetComponents(Components);
   for(auto C:Components)if(auto Mesh=C->GetStaticMesh()){
    Stairs+=Mesh->GetName()==Stair?C->GetInstanceCount():0;OldStairs+=Mesh->GetName()=="SM_Stairs"?C->GetInstanceCount():0;
    if(!Mesh->GetPathName().Contains(TEXT("/RoomKit/Meshes/"))||Mesh->GetName()=="SM_RK_portal_cap")continue;
    const FString Id=Mesh->GetName().RightChop(6);Counts.FindOrAdd(Id)+=C->GetInstanceCount();Modules+=C->GetInstanceCount();
    Materials&=C->GetNumMaterials()==5;for(int Slot=0;Slot<C->GetNumMaterials();++Slot)Materials&=C->GetMaterial(Slot)!=nullptr;
    TSet<int> UsedSlots;if(const auto* Data=Mesh->GetRenderData())if(!Data->LODResources.IsEmpty())for(const auto& Section:Data->LODResources[0].Sections)UsedSlots.Add(Section.MaterialIndex);
    SurfaceSections&=UsedSlots.Contains(0)&&UsedSlots.Contains(1)&&UsedSlots.Contains(2);
    for(int I=0;I<C->GetInstanceCount();++I){FTransform T;C->GetInstanceTransform(I,T,true);UnitScale&=T.GetScale3D().Equals(FVector::OneVector,.001f);Transforms.FindOrAdd(Id).Add(T);}
   }
  }
  bool Matching=Modules==R.RoomPlacements.Num();TMap<FString,int> Expected;
  for(const auto& P:R.RoomPlacements){++Expected.FindOrAdd(P.Id);const auto* Instances=Transforms.Find(P.Id);const FVector Location(P.X*400.f,P.Y*400.f,0);const FQuat Rotation=FRotator(0,P.Rotation*90.f,0).Quaternion();Matching&=Instances&&Instances->ContainsByPredicate([&](const FTransform& T){return T.GetLocation().Equals(Location,.1f)&&T.GetRotation().Equals(Rotation,.001f);});}
  for(const auto& Pair:Expected)Matching&=Counts.FindRef(Pair.Key)==Pair.Value;
  Check(Matching&&UnitScale&&Materials,FString::Printf(TEXT("Authored shells=%d placements=%d; exact transforms, unit scale and material slots"),Modules,R.RoomPlacements.Num()));
  Check(SurfaceSections,"Wall, floor and vault sections retain distinct material assignments");
  Check(Stairs==1&&OldStairs==0,"One themed staircase, no overlapping legacy staircase");Shot("Entrance");AfterShot=EAfterShot::Circular;break;
 }
 case 2:Shot("Circular");AfterShot=EAfterShot::Bridge;break;
 case 3:Shot("Bridge");AfterShot=EAfterShot::RoundedTurn;break;
 case 4:Shot("RoundedTurn");AfterShot=EAfterShot::BossApproach;break;
 case 5:Shot("BossApproach");AfterShot=EAfterShot::UnlockKey;break;
 case 6:
  Sweep(true);if(!D.Boss.IsEmpty()){
   Check(!M->CanDescend(),"Possessing the key does not bypass the living boss");
   for(const auto& E:R.Boundaries)if(E.Requirement=="guardian"){M->OpenBoundary(E);Check(!R.OpenDoors.Contains(E.DoorId),"Living guardian physically seals descent");}
  }
  Shot("LockedArena");AfterShot=EAfterShot::FaceSeal;break;
 case 7:Shot("GuardianSeal");AfterShot=EAfterShot::DefeatBoss;break;
 case 8:
  Sweep(true);Check(M->CanDescend(),"Legal key and guardian completion release the stairs");Check(RoomPosition("sealed_descent",FIntPoint(0,-2),2),"Authored descent chamber exists");break;
 case 9:
  Shot("SealedStairs");Check(ArchitectureBuilds==BuildsAtEntry,"Exploration and opening gates reuse the static architecture");break;
 case 10:
  ++Floor;Stage=0;if(Floor<=Last)break;
  Report+=FString::Printf(TEXT("Floors=%d; physical sweeps=%d; captured_room_types=%d; failures=%d\n"),Last-First+1,Sweeps,CapturedKinds.Num(),Failures);
  FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("RoomKit/runtime_results.txt")));
  UE_LOG(LogTemp,Display,TEXT("ROOM_KIT_REVIEW_COMPLETE floors=%d sweeps=%d failures=%d"),Last-First+1,Sweeps,Failures);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);break;
 }
}
