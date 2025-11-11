// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool; 
using System.IO;

public class EngineTest : ModuleRules
{
    public EngineTest(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // CRUCIAL: Add the plugin's Public directory to the search paths
        // This is necessary when the main game module relies on a plugin's code
        PublicIncludePaths.AddRange(new string[]
        {
            Path.Combine(ModuleDirectory, "..", "..", "Plugins", "YigsoftTest", "Source", "YigsoftTest", "Public")
        });

        // Ensure YigsoftTest is in the dependencies (you verified this state previously)
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "YigsoftTest" 
        });

        // ... rest of the file ...
    }
}