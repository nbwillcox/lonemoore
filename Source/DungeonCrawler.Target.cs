using UnrealBuildTool;
public class DungeonCrawlerTarget : TargetRules {
 public DungeonCrawlerTarget(TargetInfo Target) : base(Target) { Type=TargetType.Game; DefaultBuildSettings=BuildSettingsVersion.V7; IncludeOrderVersion=EngineIncludeOrderVersion.Latest; ExtraModuleNames.Add("DungeonCrawler"); }
}
