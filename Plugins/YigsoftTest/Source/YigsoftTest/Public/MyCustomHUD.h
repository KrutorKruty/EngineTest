// MyCustomHUD.h

#pragma once

#include "GameFramework/HUD.h" 
// Include the base class definition needed for the AHUD inheritance.

// Forward declare the external UCLASS wrapper to maintain UHT isolation
class UFrameworkWrapper;

#include "MyCustomHUD.generated.h" // MUST be the LAST include in the header

UCLASS()
class YIGSOFTTEST_API AMyCustomHUD : public AHUD
{
    GENERATED_BODY() // Error C4430/missing file is solved when the includes above are correct

public:
    AMyCustomHUD(); // Add constructor declaration if you define it in the .cpp

    // Declaration for the function you defined in the .cpp
    virtual void DrawHUD() override;
};