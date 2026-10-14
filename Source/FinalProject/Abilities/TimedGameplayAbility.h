#pragma once

#include "CoreMinimal.h"
#include "EnhancedGameplayAbility.h"
#include "TimedGameplayAbility.generated.h"

// Base class for abilities that are only active for a limited time.
// There is one instance per actor, so the ability can only be activated again once it's no longer active.
UCLASS(Abstract)
class FINALPROJECT_API UTimedGameplayAbility : public UEnhancedGameplayAbility
{
	GENERATED_BODY()

public:
	UTimedGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//Stops the effect of the ability, however the ability ended (finished, cancelled or removed)
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	//Starts the effect of the ability, returning false cancels the activation
	virtual bool StartEffect() { return true; }

	//Reverts the effect of the ability, also called when StartEffect returned false
	virtual void StopEffect() {}

	//Called when the active time is over, finishes the ability by default
	UFUNCTION()
	virtual void OnActiveTimeOver();

	//The active time in world time. Override this when the ability changes how fast the world time runs.
	virtual float GetActiveTimeInWorldTime() const { return ActiveTime; }

	//Ends the ability as finished, not cancelled
	void FinishAbility();

	// How long the ability stays active
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (ClampMin = "0.1", ForceUnits = "s"))
	float ActiveTime = 5.f;
};
