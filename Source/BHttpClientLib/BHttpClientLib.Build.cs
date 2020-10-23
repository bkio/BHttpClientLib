/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

using UnrealBuildTool;

public class BHttpClientLib : ModuleRules
{
	public BHttpClientLib(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "OpenSSL", "zlib", "BUtilities", "HTTP" });

        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});
	}
}