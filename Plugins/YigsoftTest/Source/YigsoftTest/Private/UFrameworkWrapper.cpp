// This is the complete, final version of UFrameworkWrapper.cpp with all known errors resolved.

// 1. Keep UFrameworkWrapper.h first
#include "UFrameworkWrapper.h"
#pragma comment(lib, "Framework.Release.lib") // Library linkage (optional but kept if needed)
#include "framework.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CanvasItem.h" 
// 3. External Library Includes
#include <vector>
#include "testApp.h"
#include "testBody.h"
#include "testShape.h" // Source of the required constant (likely defines MAX_EDGES globally or requires no scope)
#include "GameFramework/PlayerController.h"
// 4. Engine/API Includes (Required for DrawDebugLine, UCanvas, FText, etc.)
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/Engine.h" // Needed for GEngine
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Internationalization/Text.h"
// Added required math include for FVector, FLinearColor conversion
#include "Math/Color.h" 

using namespace test;
using namespace app;

// If MAX_EDGES is not defined globally in a header, we define it here.
#ifndef MAX_EDGES
const int MAX_EDGES = 6;
#endif 


// --- Helper Functions for Color Conversion ---
FColor ConvertExternalColor(unsigned int ExtColor)
{
    uint8 R = (uint8)((ExtColor >> 16) & 0xFF);
    uint8 G = (uint8)((ExtColor >> 8) & 0xFF);
    uint8 B = (uint8)(ExtColor & 0xFF);
    return FColor(R, G, B, 255);
}

FLinearColor ConvertExternalColorToFLinearColor(DWORD ExtColor)
{
    uint8 R = (uint8)((ExtColor >> 16) & 0xFF);
    uint8 G = (uint8)((ExtColor >> 8) & 0xFF);
    uint8 B = (uint8)(ExtColor & 0xFF);
    return FLinearColor(R / 255.0f, G / 255.0f, B / 255.0f, 1.0f);
}

// --- 1. Static Member Initialization ---
UFrameworkWrapper* UFrameworkWrapper::Instance = nullptr;

// --- Constructor ---
UFrameworkWrapper::UFrameworkWrapper()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics; // Ensure tick order if needed
    FrameworkApp = new test::App();
}

// --- NEW: EndPlay for Cleanup (CRUCIAL UNREAL LIFECYCLE) ---
void UFrameworkWrapper::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up external framework app
    if (FrameworkApp)
    {
        delete FrameworkApp;
        FrameworkApp = nullptr;
    }

    // Unset the static instance if it was this one
    if (Instance == this)
    {
        Instance = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}


// --- BeginPlay ---
void UFrameworkWrapper::BeginPlay()
{
    Super::BeginPlay();
    Instance = this;
    if (FrameworkApp)
    {
        // Init() call remains removed as it caused C2039.
    }
}

// --- GetFrameworkWrapperInstance ---
UFrameworkWrapper* UFrameworkWrapper::GetFrameworkWrapperInstance()
{
    return Instance;
}

// --- AddFrameworkBody ---
void UFrameworkWrapper::AddFrameworkBody(int ShapeType, float X, float Y, float R)
{
    if (FrameworkApp)
    {
        FrameworkApp->AddBody(ShapeType, X, Y, R);
    }
}

// --- HandleKeyInput ---
void UFrameworkWrapper::HandleKeyInput(int32 KeyCode)
{
    if (FrameworkApp)
    {
        // Assuming testApp uses OnKeyPressed to handle external input
        FrameworkApp->OnKeyPressed(KeyCode);
    }
}

// --- TickComponent ---
void UFrameworkWrapper::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (FrameworkApp)
    {
        // Assuming OnTick handles the update/integration logic
        FrameworkApp->OnTick(DeltaTime);

        // Run OnRender with a temporary frame to collect data
        app::RenderFrame CurrentFrame;
        FrameworkApp->OnRender(CurrentFrame);

        const std::vector<test::Body*>& Bodies = FrameworkApp->GetBodies();

        const float DrawDuration = 0.0f;
        const float LineThickness = 2.0f;

        for (const test::Body* Body : Bodies)
        {
            if (!Body || !Body->m_shape || !GetWorld()) continue;

            unsigned int ExternalColorValue = Body->GetColor();
            const FColor DrawColor = ConvertExternalColor(ExternalColorValue);

            float BodyX = Body->GetX();
            float BodyY = Body->GetY();
            const test::IShape* Shape = Body->m_shape;

            float ex[MAX_EDGES];
            float ey[MAX_EDGES];
            int numEdges = 0;
            Shape->ComputeEdges(ex, ey, numEdges);

            for (int i = 0; i < numEdges; ++i)
            {
                const float CurEdgeX = ex[i];
                const float CurEdgeY = ey[i];
                const float NextEdgeX = ex[(i + 1) % numEdges];
                const float NextEdgeY = ey[(i + 1) % numEdges];

                // Note: Assuming a 2D simulation mapped to X/Y plane (Z=0)
                FVector StartPoint(BodyX + CurEdgeX, BodyY + CurEdgeY, 0.0f);
                FVector EndPoint(BodyX + NextEdgeX, BodyY + NextEdgeY, 0.0f);

                DrawDebugLine(
                    GetWorld(),
                    StartPoint,
                    EndPoint,
                    DrawColor,
                    false,
                    DrawDuration,
                    0,
                    LineThickness
                );
            }
        }
    }
}


// --- DrawUI (Implementation of required rendering for UI strings) ---
void UFrameworkWrapper::DrawUI(UCanvas* Canvas, APlayerController* PlayerController)
{
    // Check for necessary pointers
    if (!Canvas || !GEngine || !GEngine->GetTinyFont() || !PlayerController)
    {
        return;
    }

    // Get the required UFont object
    UFont* DefaultFont = GEngine->GetTinyFont();

    // Resolves LNK2019 by accessing the framework instance's render frame
    const app::RenderFrame& Frame = app::Framework::GetGlobalInstance().GetRenderFrame();

    for (const app::RenderString& StringInfo : Frame.GetStrings())
    {
        // 1. Set up properties for FCanvasTextItem
        FLinearColor Color = ConvertExternalColorToFLinearColor(StringInfo.color);
        FString TextString(StringInfo.text.c_str());

        // 2. Create the FCanvasTextItem
        FCanvasTextItem TextItem(
            FVector2D(StringInfo.x, StringInfo.y), // Screen position
            FText::FromString(TextString),         // Text content
            DefaultFont,                           // Font object
            Color                                  // Text color
        );

        // Optional: Set scale (standard 1.0)
        TextItem.Scale = FVector2D(1.0f, 1.0f);

        // Use ShadowOffset for shadow effect
        TextItem.ShadowOffset = FVector2D(1.0f, 1.0f);

        // 3. Draw the item using DrawItem
        Canvas->DrawItem(TextItem);
    }

    // Reset the canvas color to default (White) after the loop finishes
    Canvas->SetDrawColor(FColor::White);
}