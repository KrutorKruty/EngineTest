// MyCustomGameMode.cpp

#include "MyCustomGameMode.h"
#include "YigsoftTest/Public/MyCustomHUD.h"

AMyCustomGameMode::AMyCustomGameMode()
{
	
	HUDClass = AMyCustomHUD::StaticClass();
}