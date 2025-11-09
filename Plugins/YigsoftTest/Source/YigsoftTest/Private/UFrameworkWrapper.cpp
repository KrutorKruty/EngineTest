#pragma once

#include "UFrameworkWrapper.h"


#include <vector>
#include "testApp.h"
#include "testBody.h"


#include "DrawDebugHelpers.h"
#include "Engine/World.h"


using namespace test;



UFrameworkWrapper::UFrameworkWrapper()
{
  
    PrimaryComponentTick.bCanEverTick = true;

    FrameworkApp = new test::App();
}

void UFrameworkWrapper::AddFrameworkBody(int ShapeType, float X, float Y, float R)
{
    if (FrameworkApp)
    {

        FrameworkApp->AddBody(ShapeType, X, Y, R);
    }
}

void UFrameworkWrapper::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (FrameworkApp)
    {
    
        FrameworkApp->OnTick(DeltaTime);

        const std::vector<test::Body*>& Bodies = FrameworkApp->GetBodies();

      
        for (const test::Body* Body : Bodies)
        {
   
            FVector WorldLocation = FVector(Body->GetX(), Body->GetY(), 0.0f);
            float Radius = Body->GetRadius();

            DrawDebugSphere(
                GetWorld(),
                WorldLocation,
                Radius,
                12,
                FColor::Red,
                false,
                0.0f
            );
        }
    }
}