using UnrealBuildTool;
public class DungeonCrawler : ModuleRules {
 public DungeonCrawler(ReadOnlyTargetRules Target) : base(Target) { bUseUnity=false; PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs; PublicDependencyModuleNames.AddRange(new[]{"Core","CoreUObject","Engine","InputCore","ApplicationCore","UMG","Slate","SlateCore","Json","JsonUtilities","RenderCore"}); }
}
