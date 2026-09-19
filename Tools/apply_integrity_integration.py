from pathlib import Path
r=Path(__file__).resolve().parents[1]/'Source/DungeonCrawler'
p=r/'DungeonRefinements.cpp';s=p.read_text();a=s.index('void UDungeonModel::RandomizeLayouts()');s=s[:a]+'''void UDungeonModel::RandomizeLayouts(){
 for(int F=0;F<Floors.Num();++F){FString Error;bool Valid=false;for(int Attempt=0;Attempt<3&&!Valid;++Attempt){GenerateFloor(F,State->RandomSeed+F*7919+Attempt*104729);Valid=ValidateFloor(F,Error);}if(!Valid){GenerateFloor(F,73519);if(!ValidateFloor(F,Error)){UE_LOG(LogTemp,Fatal,TEXT("Validated floor fallback failed: %s"),*Error);}}}
}
''';a=s.index('bool UDungeonModel::CanDescend()');b=s.index('\nint32 UDungeonModel::SkillDamage',a);s=s[:a]+'''bool UDungeonModel::CanDescend()const{const auto& D=Floors[State->Floor];const auto& R=State->Floors[State->Floor];bool Guardian=true;if(!D.Boss.IsEmpty())for(int Y=0;Y<D.Rows.Num();++Y)for(int X=0;X<D.Rows[Y].Len();++X)if(D.Rows[Y][X]=='B'&&!R.Defeated.Contains(Cell(X,Y)))Guardian=false;bool Gate=false;for(const auto& E:R.Boundaries)if(E.Mandatory&&R.OpenDoors.Contains(E.DoorId))Gate=true;return Guardian&&Gate;}
'''+s[b:];p.write_text(s)
p=r/'DungeonExploration.cpp';s=p.read_text();a=s.index('void UDungeonModel::Reveal()');b=s.index('void UDungeonModel::EnterFloor',a);s=s[:a]+s[b:];a=s.index('bool UDungeonModel::Walkable');b=s.index('\nbool UDungeonModel::IsDefeated',a);s=s[:a]+'''bool UDungeonModel::Walkable(int32 X,int32 Y)const{return Tile(X,Y)!='#'&&CanCross(State->Floor,Cell(State->X,State->Y),Cell(X,Y));}'''+s[b:];s=s.replace('if(Combat||Screen!="Dungeon")return false;int32 F=', 'if(Combat||Screen!="Dungeon"||(Direction!=1&&Direction!=-1))return false;int32 F=');s=s.replace('if(Combat||!Floors.IsValidIndex(FloorIndex))return;', 'if(Combat||!Floors.IsValidIndex(FloorIndex))return;DoorOpening.Empty();');s=s.replace("int32 C=Cell(X,Y);TCHAR T=Tile(X,Y);\n if(State->CorpseFloor", "int32 C=Cell(X,Y);TCHAR T=Tile(X,Y);\n if(X!=State->X||Y!=State->Y){const auto* E=Boundary(State->Floor,Cell(State->X,State->Y),C);if(!E||E->Kind==\"Wall\"){Say(\"Solid stone blocks the way.\");return;}if(!CanCross(State->Floor,Cell(State->X,State->Y),C)){OpenBoundary(*E);return;}if(T=='>'||T=='A'){Say(\"Step onto the stairs to descend.\");return;}}\n if(State->CorpseFloor")
# Lever/plate activation operates the explicit linked group; never opens unrelated key locks.
start='if(T==\'P\'){';a=s.index(start);b=s.index('\n if(T==\'S\'',a);s=s[:a]+'''if(T=='P'){R.Switches.AddUnique(C);for(const auto& Edge:R.Boundaries)if(Edge.Requirement=="switch")OpenBoundary(Edge);WorldDirty=true;}
'''+s[b:];a=s.index("if(T=='L'){for(");b=s.index('\n if(R.Taken.Contains(C))',a);s=s[:a]+'''if(T=='L'){R.Switches.AddUnique(C);R.Taken.AddUnique(C);for(const auto& Edge:R.Boundaries)if(Edge.Requirement=="switch")OpenBoundary(Edge);Reveal();Say("A chain strains. The iron gate rises.");return;}
'''+s[b:];s=s.replace('R.Taken.Add(C);WorldDirty=true;Say("Obtained', 'R.Taken.Add(C);WorldDirty=true;Reveal();Say("Obtained');s=s.replace('WorldDirty=true;Say(T==\'$\'', 'WorldDirty=true;Reveal();Say(T==\'$\'');p.write_text(s)
p=r/'DungeonModel.cpp';s=p.read_text();s=s.replace('Floors=BaseFloors;if(!Testing)RandomizeLayouts();','Floors=BaseFloors;RandomizeLayouts();');s=s.replace('Loaded->Schema!=1','(Loaded->Schema<1||Loaded->Schema>2)');a=s.index(' if(Loaded->Y<0');b=s.index('\n if(Loaded->Hardcore)',a);s=s[:a]+''' if(Loaded->Facing<0||Loaded->Facing>3)return false;
 // Validate the candidate and migrate in memory; never rewrite the source save on load.
 auto PriorState=State;auto PriorFloors=Floors;State=Loaded;Floors=BaseFloors;FString TopologyError;
 bool TopologyOK=RestoreTopology(TopologyError);const auto& Rows=Floors[Loaded->Floor].Rows;
 TopologyOK=TopologyOK&&Rows.IsValidIndex(Loaded->Y)&&Loaded->X>=0&&Loaded->X<Rows[Loaded->Y].Len()&&Rows[Loaded->Y][Loaded->X]!='#';
 State=PriorState;Floors=PriorFloors;if(!TopologyOK){Say("Save layout rejected: "+TopologyError);return false;}
'''+s[b:];s=s.replace('WorldDirty=true;Say("Journey restored.")','WorldDirty=true;DoorOpening.Empty();Reveal();Say("Journey restored.")');p.write_text(s)
'''
'''
