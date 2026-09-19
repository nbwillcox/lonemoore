using UnrealBuildTool;
public class DungeonCrawlerEditorTarget : TargetRules {
 public DungeonCrawlerEditorTarget(TargetInfo Target) : base(Target) { Type=TargetType.Editor; DefaultBuildSettings=BuildSettingsVersion.V7; IncludeOrderVersion=EngineIncludeOrderVersion.Latest; ExtraModuleNames.Add("DungeonCrawler"); }
}
