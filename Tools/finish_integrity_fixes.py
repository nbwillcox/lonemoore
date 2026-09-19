from pathlib import Path
r=Path(__file__).resolve().parents[1]/'Source/DungeonCrawler'
p=r/'DungeonGame.cpp';s=p.read_text();s=s.replace('auto A=GetWorld()->SpawnActor<AStaticMeshActor>();A->SetMobility(EComponentMobility::Movable);A->GetStaticMeshComponent()->SetStaticMesh(Plane);','auto A=GetWorld()->SpawnActor<AStaticMeshActor>();A->SetMobility(EComponentMobility::Movable);A->SetActorEnableCollision(false);A->GetStaticMeshComponent()->SetStaticMesh(Plane);');p.write_text(s)
p=r/'DungeonTests.cpp';s=p.read_text();s=s.replace('  TestTrue(TEXT("Reach Astra"),Travel(M,FindTile(M,\'A\')));','''  // Prepare through ordinary town services before the final fight; the former harness
  // skipped its final rest and never upgraded equipment after the Phase 2 boss changes.
  M->Town();M->TownService("resurrect");M->TownService("rest");
  for(int H=0;H<M->State->Party.Num();++H)for(int Q=0;Q<3;++Q){M->Upgrade(H,0);M->Upgrade(H,3);}
  M->Waypoint(M->State->SelectedWaypoint);
  TestTrue(TEXT("Reach Astra"),Travel(M,FindTile(M,'A')));''');p.write_text(s)
p=r/'DungeonUI.cpp';s=p.read_text();s=s.replace('P.Text("N",1433,23,17,Gold);','P.Frame(1308,43,264,264);P.Text("N",1433,23,17,Gold);');p.write_text(s)
