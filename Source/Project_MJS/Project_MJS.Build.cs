// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_MJS : ModuleRules
{
	public Project_MJS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"FMODStudio",
			"LevelSequence",
			"MovieScene",
			"MovieSceneTracks",
			"CinematicCamera",
			"Niagara",
			"AIModule",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraphRuntime", "FMODStudioEditor" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
