#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FinalProjectGameMode.generated.h"

class APlayerCharacter;

// Spawns the PlayerCharacter. Make a Blueprint child and set its Default Pawn Class to the Blueprint
// child of the PlayerCharacter, that's where the input assets get assigned.
UCLASS()
class FINALPROJECT_API AFinalProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFinalProjectGameMode();

	/**
	* Ends the game once the player reached the finish. Quits the game by default,
	* override it in the Blueprint child to show a win screen instead.
	* @param player the player that reached the finish.
	**/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Game")
	void FinishGame(APlayerCharacter* player);
};
