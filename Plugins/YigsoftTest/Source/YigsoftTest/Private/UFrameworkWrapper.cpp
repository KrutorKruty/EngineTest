#pragma once

#include "UFrameworkWrapper.h"


#include <vector>
#include "testApp.h"
#include "testBody.h"
#include "testShape.h"


#include "DrawDebugHelpers.h"
#include "Engine/World.h"


using namespace test;

// Define the maximum number of edges the shapes in your framework can have
const int MAX_EDGES = 6;


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

        // Use the color set in the Unreal Editor (from UFrameworkWrapper.h UPROPERTY)
        // If DebugDrawColor is not yet defined in the header, use a default FColor::White
        const FColor CurrentDrawColor = DebugDrawColor;

        for (const test::Body* Body : Bodies)
        {
            // Ensure the body and its shape are valid before attempting to draw
            if (!Body || !Body->m_shape || !GetWorld()) continue;

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

                // Draw the line segment using the editor-exposed color
                DrawDebugLine(
                    GetWorld(),
                    StartPoint,
                    EndPoint,
                    CurrentDrawColor, // <--- Using the color exposed to the Editor
                    false, // persistent
                    0.0f,  // life time (one frame)
                    0,     // depth priority
                    2.0f   // thickness
                );
            }
        }
    }
}