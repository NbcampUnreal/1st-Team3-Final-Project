// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Zone064 : ModuleRules
{
	public Zone064(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "OnlineSubsystem", "OnlineSubsystemSteam", "OnlineSubsystemUtils", "NetCore", "Slate", "SlateCore", "Voice"});

		PrivateDependencyModuleNames.AddRange(new string[] { "OnlineSubsystemSteam" });

        //PublicIncludePaths.AddRange(new string[] { "Zone064", });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
