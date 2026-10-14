#pragma once

#include "CoreMinimal.h"
#include "Abilities/TimedGameplayAbility.h"
#include "TimeSlowdownAbility.generated.h"

// Slows down the time of the world for the active time, without affecting the player.
// Everything that moves with the world time slows down, like the Crusher, so the player can pass under it.
UCLASS()
class FINALPROJECT_API UTimeSlowdownAbility : public UTimedGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual bool StartEffect() override;
	virtual void StopEffect() override;

	//The ability timer runs on the slowed down world time, so it has to wait less world time
	virtual float GetActiveTimeInWorldTime() const override;

	// How fast the world runs while the ability is active, 0.25 means a quarter of the normal speed
	UPROPERTY(EditDefaultsOnly, Category = "Time Slowdown", meta = (ClampMin = "0.05", ClampMax = "1"))
	float WorldTimeDilation = 0.25f;

private:
	float AppliedWorldTimeDilation = 1.f;
	float PreviousWorldTimeDilation = 1.f;
	float PreviousAvatarTimeDilation = 1.f;
	bool bIsTimeSlowedDown = false;
};
