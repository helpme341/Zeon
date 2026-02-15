// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Logic : ModuleRules
{
	public Logic(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Zeon",
				"CoreUObject",
				"Engine",
			}
		);
	}
}