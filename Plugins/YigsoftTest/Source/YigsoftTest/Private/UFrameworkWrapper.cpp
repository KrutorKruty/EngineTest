// UFrameworkWrapper.cpp

#include "UFrameworkWrapper.h"

#include <vector>
// Include external framework headers to get full class definitions
#include "testApp.h"
#include "testBody.h"
#include "testShape.h"
#include "../external/framework/include/framework.h" // Assuming this is needed for the framework types

#include "DrawDebugHelpers.h"
#include "Engine/World.h"


using namespace test;

// Define the maximum number of edges the shapes in your framework can have
const int MAX_EDGES = 6;

// --- Helper Function to Convert External Color Format ---
// This function converts the external framework's color format (e.g., 0x00BBGGRR)
// into Unreal Engine's FColor (RGBA).
FColor ConvertExternalColor(unsigned int ExtColor)
{
    // External format is assumed to be 0x00BBGGRR (WinAPI RGB)
    uint8 R = (uint8)(ExtColor & 0xFF);
    uint8 G = (uint8)((ExtColor >> 8) & 0xFF);
    uint8 B = (uint8)((ExtColor >> 16) & 0xFF);
    // Set Alpha to opaque (255)
    return FColor(R, G, B, 255);
}
// --------------------------------------------------------


UFrameworkWrapper::UFrameworkWrapper()
{
    // Ensure the component starts ticking every frame
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize the external physics application instance
    FrameworkApp = new test::App();
}

void UFrameworkWrapper::AddFrameworkBody(int ShapeType, float X, float Y, float R)
{
    if (FrameworkApp)
    {
        FrameworkApp->AddBody(ShapeType, X, Y, R);
    }
}

void UFrameworkWrapper::HandleKeyInput(int32 KeyCode)
{
    if (FrameworkApp)
    {
        // Forward the key code received from the Unreal input system 
        // to the external physics simulation's key handler.
        FrameworkApp->OnKeyPressed(KeyCode);
    }
}


void UFrameworkWrapper::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (FrameworkApp)
    {
        // Run the external physics simulation step
        FrameworkApp->OnTick(DeltaTime);

        // Get the list of all bodies from the simulation
        const std::vector<test::Body*>& Bodies = FrameworkApp->GetBodies();

        // The debug lines will only live for one frame
        const float DrawDuration = 0.0f;
        const float LineThickness = 2.0f;

        for (const test::Body* Body : Bodies)
        {
            // Ensure the body and its shape are valid before attempting to draw
            // The check for Body->m_shape implies 'm_shape' is a public member.
            if (!Body || !Body->m_shape || !GetWorld()) continue;

            // --- COLOR FUNCTIONALITY REMOVED AND REVERTED ---
            // 1. Get the color value from the external body object.
            //    This assumes 'test::Body' has a public method 'GetColor()'.
            unsigned int ExternalColorValue = Body->GetColor();

            // 2. Convert the external color format to Unreal's FColor.
            const FColor DrawColor = ConvertExternalColor(ExternalColorValue);
            // ----------------------------------------------------

            // 1. Get position and shape data
            float BodyX = Body->GetX();
            float BodyY = Body->GetY();
            const test::IShape* Shape = Body->m_shape;

            // 2. Compute the edges of the shape (vertices relative to the body's center)
            float ex[MAX_EDGES];
            float ey[MAX_EDGES];
            int numEdges = 0;
            Shape->ComputeEdges(ex, ey, numEdges);

            // 3. Draw the shape edges using DrawDebugLine
            for (int i = 0; i < numEdges; ++i)
            {
                // Current edge point (relative to body center)
                const float CurEdgeX = ex[i];
                const float CurEdgeY = ey[i];

                // Next edge point (wrapping around to close the shape)
                const float NextEdgeX = ex[(i + 1) % numEdges];
                const float NextEdgeY = ey[(i + 1) % numEdges];

                // Global Start Point (Body position + local edge point)
                FVector StartPoint(BodyX + CurEdgeX, BodyY + CurEdgeY, 0.0f);

                // Global End Point
                FVector EndPoint(BodyX + NextEdgeX, BodyY + NextEdgeY, 0.0f);

                // Draw the line segment using the color retrieved from the 'test::Body'
                DrawDebugLine(
                    GetWorld(),
                    StartPoint,
                    EndPoint,
                    DrawColor, // <--- Using the color from the external body
                    false,
                    DrawDuration,
                    0,
                    LineThickness
                );
            }
        }
    }
}