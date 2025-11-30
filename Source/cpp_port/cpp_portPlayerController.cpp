// Copyright Epic Games, Inc. All Rights Reserved.


#include "cpp_portPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "cpp_portCameraManager.h"

Acpp_portPlayerController::Acpp_portPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = Acpp_portCameraManager::StaticClass();
}

void Acpp_portPlayerController::SetupInputComponent()
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
