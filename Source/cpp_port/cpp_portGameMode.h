// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "cpp_portGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class Acpp_portGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	Acpp_portGameMode();
};



