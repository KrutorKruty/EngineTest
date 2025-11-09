// Copyright Epic Games, Inc. All Rights Reserved.

#include "YigsoftTest.h"
#include "testApp.h"

#define LOCTEXT_NAMESPACE "FYigsoftTestModule"

void FYigsoftTestModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FYigsoftTestModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FYigsoftTestModule, YigsoftTest)

#ifdef _DEBUG
	#pragma comment (lib, "framework.Debug.lib")
#else
	#pragma comment (lib, "framework.Release.lib")
	#pragma comment (lib, "reference.Release.lib")
#endif

int YigsoftTest()
{
	std::unique_ptr< app::Framework > framework( new app::Framework() );

#ifndef _DEBUG
	extern app::IApp* CreateRefrenceApp();
	framework->RegisterApp( CreateRefrenceApp() );
#endif

	framework->RegisterApp( new test::App() );

	if ( !framework->Init() )
		return -1;

	framework->Loop();
	return 0;
}