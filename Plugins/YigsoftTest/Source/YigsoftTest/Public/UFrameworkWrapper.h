#pragma once

// Forward Declarations for Unreal types used in the UI function
class UCanvas;
class APlayerController;
class UActorComponent; // Required forward declaration since you're removing the full include of ActorComponent.h

// --- FORWARD DECLARATIONS FOR EXTERNAL FRAMEWORK (Fix to avoid UHT crash) ---
namespace app {
	class RenderFrame;
	class Framework;
	struct RenderString;
}
// --------------------------------------------------------------------------

namespace test { class App; }

#include "UFrameworkWrapper.generated.h" // UHT generated code must come after forward declarations

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YIGSOFTTEST_API UFrameworkWrapper : public UActorComponent
{
	GENERATED_BODY()

public:
	UFrameworkWrapper();

	// --- Existing Public Functions ---
	void AddFrameworkBody(int ShapeType, float X, float Y, float R);
	void HandleKeyInput(int32 KeyCode);

	// --- NEW: Static Accessor for the active Framework instance ---
	// This allows the custom AHUD class to access the framework data for UI drawing.
	static UFrameworkWrapper* GetFrameworkWrapperInstance();

	// --- NEW: Accessor for the external App (used for data, like GetRenderFrame) ---
	test::App* GetFrameworkApp() const { return FrameworkApp; }

	// --- NEW: Custom UI Draw Hook (Requires an AHUD class to call this) ---
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