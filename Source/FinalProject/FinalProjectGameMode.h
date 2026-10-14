#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FinalProjectGameMode.generated.h"

// Spawns the PlayerCharacter. Make a Blueprint child and set its Default Pawn Class to the Blueprint
// child of the PlayerCharacter, that's where the input assets get assigned.
UCLASS()
class FINALPROJECT_API AFinalProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFinalProjectGameMode();
};
