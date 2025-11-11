#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h" // Required for UActorComponent base class
#include "UFrameworkWrapper.generated.h" 

// Forward Declarations for Unreal types used in the UI function
class UCanvas;
class APlayerController;

// --- FORWARD DECLARATIONS FOR EXTERNAL FRAMEWORK ---
namespace app {
	class RenderFrame;
	class Framework;
	struct RenderString;
}
// ----------------------------------------------------

namespace test { class App; }


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YIGSOFTTEST_API UFrameworkWrapper : public UActorComponent
{
	GENERATED_BODY()

public:
	UFrameworkWrapper();

	// --- Existing Public Functions ---
    // *** FIX: ADDED UFUNCTION MACROS to expose to Blueprint ***
	UFUNCTION(BlueprintCallable, Category = "Framework")
	void AddFrameworkBody(int ShapeType, float X, float Y, float R);

    // *** FIX: ADDED UFUNCTION MACROS to expose to Blueprint ***
	UFUNCTION(BlueprintCallable, Category = "Framework")
	void HandleKeyInput(int32 KeyCode);

	// --- Static Accessor for the active Framework instance ---
	static UFrameworkWrapper* GetFrameworkWrapperInstance();

	// --- Accessor for the external App (used for data, like GetRenderFrame) ---
	test::App* GetFrameworkApp() const { return FrameworkApp; }

	// --- Custom UI Draw Hook (Requires an AHUD class to call this) ---
	void DrawUI(UCanvas* Canvas, APlayerController* PC);

protected:
	virtual void BeginPlay() override;

	// --- IMPLEMENTATION ADDED: RESOLVES C2509 ERROR ---
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisFunction) override;

private:
	test::App* FrameworkApp;

	// --- NEW: Static Instance Pointer ---
	static UFrameworkWrapper* Instance;
};