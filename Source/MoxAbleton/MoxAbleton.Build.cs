// Copyright Ars Electronica Futurelab. All Rights Reserved.

using UnrealBuildTool;

public class MoxAbleton : ModuleRules
{
	public MoxAbleton(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"OSC",        // UE OSC Plugin for OSCStream
				"Sockets",    // UDP socket communication
				"Networking", // FInternetAddr
				"MoxUtility"
            }
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
			);
	}
}
