// Copyright Epic Games, Inc. All Rights Reserved.


#include "PluginZeonPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "PluginZeonCameraManager.h"

APluginZeonPlayerController::APluginZeonPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = APluginZeonCameraManager::StaticClass();
}

void APluginZeonPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}
