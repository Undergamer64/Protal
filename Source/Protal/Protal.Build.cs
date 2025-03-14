// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Protal : ModuleRules
{
	public Protal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
