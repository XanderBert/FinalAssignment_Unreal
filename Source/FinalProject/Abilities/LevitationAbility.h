#pragma once

#include "CoreMinimal.h"
#include "Abilities/TimedGameplayAbility.h"
#include "LevitationAbility.generated.h"

class UCharacterMovementComponent;

// Moves the player up by LevitationHeight and keeps them there for the active time, the player can still move around.
// Afterwards the player is moved back to the height they had when the levitation was triggered.
// LevitationHeight should be higher than the Step, but too low to fly over the Crusher.
UCLASS()
class FINALPROJECT_API ULevitationAbility : public UTimedGameplayAbility
{
	GENERATED_BODY()

public:
	ULevitationAbility();

protected:
	virtual bool StartEffect() override;
	virtual void StopEffect() override;

	//Doesn't end the ability yet, the player first has to be moved back to their original height
	virtual void OnActiveTimeOver() override;

	virtual bool CanTick_Implementation() const override;
	virtual void TickLogic_Implementation(float deltaTime) override;

	// How far the player is moved up
	UPROPERTY(EditDefaultsOnly, Category = "Levitation", meta = (ClampMin = "0", ForceUnits = "cm"))
	float LevitationHeight = 200.f;

	// How fast the player is moved up and back down
	UPROPERTY(EditDefaultsOnly, Category = "Levitation", meta = (ClampMin = "1", ForceUnits = "cm/s"))
	float VerticalSpeed = 400.f;

private:
	enum class ELevitationPhase : uint8
	{
		None,
		Rising,
		Hovering,
		Lowering,
	};

	//Moves the player vertically towards the target height, returns true once it's reached or something blocks the player
	bool MoveTowardsHeight(float targetHeight, float deltaTime);

	UCharacterMovementComponent* GetCharacterMovement() const;

	ELevitationPhase Phase = ELevitationPhase::None;
	float OriginalHeight = 0.f;
};
