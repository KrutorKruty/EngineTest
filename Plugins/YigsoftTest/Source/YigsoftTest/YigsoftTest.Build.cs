// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class YigsoftTest : ModuleRules
{
    public YigsoftTest(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        // --- Path Definitions ---
        string PluginRoot = this.PluginDirectory;
        string FrameworkRoot = Path.Combine(PluginRoot, "external", "framework");
        string IncludePath = Path.Combine(FrameworkRoot, "include");

        // CHANGED: Using lowercase "lib" as requested for case-sensitive path handling.
        string LibPath = Path.Combine(FrameworkRoot, "lib");

        // Removed redundant Directory.Exists check here.

        // --- Include Paths ---
        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Public"),
                IncludePath
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Private"),
                IncludePath
            }
        );


        // --- Public Dependency Modules (Combined and Cleaned) ---
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "InputCore",
                "DeveloperSettings",
                "RenderCore",
                "Engine",
                "UMG",
                "SlateCore",
                "Slate",
                "Renderer",
                "ApplicationCore",
                // FIX: Adding RHI (Render Hardware Interface) to resolve deep UCanvas linkage errors.
                "RHI"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
				// Keep your private dependencies here
			}
        );

        // --- External Library Linking (Corrected Syntax for /WHOLEARCHIVE) ---

        // 1. Removed PublicSystemLibraryPaths.Add(LibPath); to rely ONLY on absolute paths below.

        // 2. Determine which specific library file to link based on the Unreal build configuration.
        string FrameworkLibraryFileName = "";
        string ReferenceLibraryFileName = "";

        bool bIsReleaseConfig = Target.Configuration == UnrealTargetConfiguration.Development ||
                                Target.Configuration == UnrealTargetConfiguration.Shipping ||
                                Target.Configuration == UnrealTargetConfiguration.Test;

        if (bIsReleaseConfig)
        {
            // If Unreal's configuration is Development or Release, use the external Release libraries.
            FrameworkLibraryFileName = "Framework.Release.lib";
            ReferenceLibraryFileName = "reference.Release.lib";
        }
        else
        {
            // Otherwise (Debug), use the external Debug libraries.
            FrameworkLibraryFileName = "Framework.Debug.lib";
            ReferenceLibraryFileName = "reference.Debug.lib";
        }

        // --- Framework Library (Requires /WHOLEARCHIVE) ---
        string AbsoluteFrameworkLibraryPath = Path.Combine(LibPath, FrameworkLibraryFileName);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // Link Framework.lib with /WHOLEARCHIVE
            PublicAdditionalLibraries.Add("/WHOLEARCHIVE:" + AbsoluteFrameworkLibraryPath);
        }
        else
        {
            PublicAdditionalLibraries.Add(AbsoluteFrameworkLibraryPath);
        }


        // --- REFERENCE Library (Fix for LNK1104) ---
        string AbsoluteReferenceLibraryPath = Path.Combine(LibPath, ReferenceLibraryFileName);

        // Link reference.lib normally (it typically doesn't need /WHOLEARCHIVE)
        PublicAdditionalLibraries.Add(AbsoluteReferenceLibraryPath);


        DynamicallyLoadedModuleNames.AddRange(new string[] { });
    }
}