// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO; 

public class YigsoftTest : ModuleRules
{
	public YigsoftTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false; 


        string PluginRoot = this.PluginDirectory; 
        
        string FrameworkRoot = Path.Combine(PluginRoot, "external", "framework");

        string IncludePath = Path.Combine(FrameworkRoot, "include");
        string LibPath = Path.Combine(FrameworkRoot, "Lib"); 


		PublicIncludePaths.AddRange(
			new string[] {
				IncludePath 
			}
		);
				
		PrivateIncludePaths.AddRange(
			new string[] {
                Path.Combine(ModuleDirectory, "Private"), 
				IncludePath
			}
		);
			

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"Engine", 
				"CoreUObject", 
				"Slate", 
				"SlateCore", 
				"InputCore",
                "RenderCore", 
                "DeveloperSettings",
        

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
        

        
        PublicSystemLibraryPaths.Add(LibPath); 
        
        if (Target.Configuration == UnrealTargetConfiguration.Development || Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            PublicAdditionalLibraries.Add("framework.Release.lib"); 
        }
        else 
        {
            PublicAdditionalLibraries.Add("framework.Release.lib"); 
        }


		
		DynamicallyLoadedModuleNames.AddRange(new string[] {});
	}
}

