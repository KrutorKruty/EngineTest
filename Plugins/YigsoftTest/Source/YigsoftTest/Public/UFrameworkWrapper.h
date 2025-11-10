// UFrameworkWrapper.h

#pragma once

#include "Components/ActorComponent.h"
#include "UFrameworkWrapper.generated.h"

// Forward declare your test::App to use it in the class
namespace test { class App; }

UCLASS(ClassGroup = (YigsoftTest), meta = (BlueprintSpawnableComponent))
class YIGSOFTTEST_API UFrameworkWrapper : public UActorComponent
{
    GENERATED_BODY()

public:
    UFrameworkWrapper();

protected:
    // Ptr to your external framework's application logic
    test::App* FrameworkApp;

public:
    // REMOVED: UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Visuals")
    // REMOVED: FColor DebugDrawColor = FColor::White;

    // UFUNCTIONs... (existing code)
    UFUNCTION(BlueprintCallable, Category = "Framework Functions")
    void AddFrameworkBody(int ShapeType, float X, float Y, float R);

    UFUNCTION(BlueprintCallable, Category = "Framework Functions")
    void HandleKeyInput(int32 KeyCode);

    // Call OnTick from Unreal's Tick function
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};