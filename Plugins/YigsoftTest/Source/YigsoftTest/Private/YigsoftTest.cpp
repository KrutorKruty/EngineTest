// Copyright Epic Games, Inc. All Rights Reserved.

#include "YigsoftTest.h"

// ----------------------------------------------------------------------------
// CRITICAL FIX: Ensure 'app.h' and 'Framework.h' are included first
// so the IApp interface is known globally before CreateRefrenceApp is declared.
// ----------------------------------------------------------------------------
#include "app.h" 
#include "Framework.h" 

#include "testApp.h" // Assuming this defines test::App
// --- Standard C++ Headers for Fix ---
#include <memory> 
#include <string> 

#define LOCTEXT_NAMESPACE "FYigsoftTestModule"

// ----------------------------------------------------------------------------
// Unreal Module Implementation
// ----------------------------------------------------------------------------

void FYigsoftTestModule::StartupModule()
{
    // Called after your module is loaded into memory
    UE_LOG(LogTemp, Log, TEXT("YigsoftTest module started up."));
}

void FYigsoftTestModule::ShutdownModule()
{
    // Called during module shutdown for cleanup
    UE_LOG(LogTemp, Log, TEXT("YigsoftTest module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FYigsoftTestModule, YigsoftTest)

// ----------------------------------------------------------------------------
// External Library Linking
// ----------------------------------------------------------------------------

#ifdef _DEBUG
#pragma comment(lib, "framework.Debug.lib")
#else
#pragma comment(lib, "framework.Release.lib")
#pragma comment(lib, "reference.Release.lib")
#endif

// ----------------------------------------------------------------------------
// Entry Function: YigsoftTest()
// ----------------------------------------------------------------------------

int YigsoftTest()
{
    // Create the framework
    // Fix: std::unique_ptr and std::make_unique require the <memory> header.
    std::unique_ptr<app::Framework> framework = std::make_unique<app::Framework>();

#ifndef _DEBUG
    // External function provided by reference library (or our stub)
    // FIX: Using app::IApp explicitly since it's in a namespace.
    extern app::IApp* CreateRefrenceApp();
    // FIX 2b: Called the function in the global namespace
    framework->RegisterApp(CreateRefrenceApp());
#endif

    // Register local test app
    framework->RegisterApp(new test::App());

    // Initialize and run the framework
    if (!framework->Init())
    {
        UE_LOG(LogTemp, Error, TEXT("Framework initialization failed."));
        return -1;
    }

    framework->Loop();
    return 0;
}

// ----------------------------------------------------------------------------
// Stub or Real Implementation of CreateRefrenceApp()
// ----------------------------------------------------------------------------

// FIX: The return type app::IApp* is now correctly qualified with the namespace.
app::IApp* CreateRefrenceApp()
{
    UE_LOG(LogTemp, Warning, TEXT("Stub CreateRefrenceApp() called. Returning nullptr."));
    // TODO: Replace with a real app instance if available:
    // return new ReferenceApp();

    return nullptr;
}