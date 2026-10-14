#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilityTask_SafeWaitGameplayEvent.generated.h"

UCLASS()
class ABILITYSYSTEM_API UAbilityTask_SafeWaitGameplayEvent : public UAbilityTask_WaitGameplayEvent
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_SafeWaitGameplayEvent* SafeWaitGameplayEvent(UGameplayAbility* owningAbility, FGameplayTag eventTag, AActor* optionalTarget = nullptr, bool onlyTriggerOnce = false, bool onlyMatchExact = true, float maxWaitTime = 10.0f, bool breakOnTimeLimit = false);

	virtual void Activate() override;

protected:
	//Both the exact and the nested tag events end up here
	virtual void GameplayEventContainerCallback(FGameplayTag MatchingTag, const FGameplayEventData* Payload) override;
	virtual void OnDestroy(bool AbilityEnding) override;

private:
	UFUNCTION()
	void SafeTimerCallback();

	void StartTimer();
	void ClearTimer();

	TWeakObjectPtr<AActor> OwningActor = nullptr;
	float MaxWaitTime = 0.0f;
	bool BreakOnTimeLimit = false;
	FTimerHandle TimerHandle{};
};
