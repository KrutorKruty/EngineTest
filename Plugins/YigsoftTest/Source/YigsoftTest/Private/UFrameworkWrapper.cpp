// This is the complete, final version of UFrameworkWrapper.cpp with all known errors resolved.

// 1. Keep UFrameworkWrapper.h first
#include "UFrameworkWrapper.h"
#pragma comment(lib, "Framework.Release.lib")
#include "framework.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CanvasItem.h" 
// 3. External Library Includes (Safe to be here, as UHT ignores .cpp files)
#include <vector>
#include "testApp.h"
#include "testBody.h"
#include "testShape.h"
#include "../external/framework/include/framework.h"
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

const int MAX_EDGES = 6;

// --- 1. Static Member Initialization (REQUIRED) ---
UFrameworkWrapper* UFrameworkWrapper::Instance = nullptr;

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

// --- Constructor (WORKING LINES) ---
UFrameworkWrapper::UFrameworkWrapper()
{
    PrimaryComponentTick.bCanEverTick = true;
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


// --- BeginPlay (WORKING LINES) ---
void UFrameworkWrapper::BeginPlay()
{
    Super::BeginPlay();
    Instance = this;
}

// --- GetFrameworkWrapperInstance (IMPLEMENTATION ADDED) ---
UFrameworkWrapper* UFrameworkWrapper::GetFrameworkWrapperInstance()
{
    return Instance;
}

// --- AddFrameworkBody (WORKING LINES) ---
void UFrameworkWrapper::AddFrameworkBody(int ShapeType, float X, float Y, float R)
{
    if (FrameworkApp)
    {
        FrameworkApp->AddBody(ShapeType, X, Y, R);
    }
}

// --- HandleKeyInput (WORKING LINES) ---
void UFrameworkWrapper::HandleKeyInput(int32 KeyCode)
{
    if (FrameworkApp)
    {
        FrameworkApp->OnKeyPressed(KeyCode);
    }
}

// --- TickComponent (WORKING LINES) ---
void UFrameworkWrapper::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (FrameworkApp)
    {
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


// --- DrawUI (IMPLEMENTATION ADDED to resolve LNK2019 and C2039) ---
void UFrameworkWrapper::DrawUI(UCanvas* Canvas, APlayerController* PlayerController)
{
    // Check for necessary pointers
    if (!Canvas || !GEngine || !GEngine->GetTinyFont() || !PlayerController)
    {
        return;
    }

    // Get the required UFont object (GEngine->GetTinyFont() is the modern way)
    UFont* DefaultFont = GEngine->GetTinyFont();

    // Resolves LNK2019: GetGlobalInstance() and provides the RenderFrame data
    const app::RenderFrame& Frame = app::Framework::GetGlobalInstance().GetRenderFrame();

    for (const app::RenderString& StringInfo : Frame.GetStrings())
    {
        // 1. Set up properties for FCanvasTextItem
        FLinearColor Color = ConvertExternalColorToFLinearColor(StringInfo.color);
        FString TextString(StringInfo.text.c_str());

        // 2. Create the FCanvasTextItem
        FCanvasTextItem TextItem(
            FVector2D(StringInfo.x, StringInfo.y), // Screen position
            FText::FromString(TextString),          // Text content
            DefaultFont,                            // Font object
            Color                                   // Text color
        );

        // Optional: Set scale
        TextItem.Scale = FVector2D(1.0f, 1.0f);

        // FIX: The bEnableShadow property might be deprecated or unavailable in this engine version.
        // We set the ShadowOffset instead, which achieves the same shadow effect.
        TextItem.ShadowOffset = FVector2D(1.0f, 1.0f);

        // 3. Draw the item using DrawItem. This is the more stable/modern way
        // to draw UI elements to the Canvas and avoids the LNK2019 error 
        // that often plagues the DrawText overloads.
        Canvas->DrawItem(TextItem);
    }

    // Reset the canvas color to default (White) after the loop finishes
    Canvas->SetDrawColor(FColor::White);
}