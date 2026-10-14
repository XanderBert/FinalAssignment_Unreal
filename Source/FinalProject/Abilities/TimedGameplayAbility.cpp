#include "Abilities/TimedGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

UTimedGameplayAbility::UTimedGameplayAbility()
{
	//With one instance per actor the ability system refuses to activate the ability while it's active
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UTimedGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !StartEffect())
	{
		constexpr bool bReplicateEndAbility = true;
		constexpr bool bWasCancelled = true;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		return;
	}

	UAbilityTask_WaitDelay* WaitActiveTime = UAbilityTask_WaitDelay::WaitDelay(this, GetActiveTimeInWorldTime());
	WaitActiveTime->OnFinish.AddDynamic(this, &UTimedGameplayAbility::OnActiveTimeOver);
	WaitActiveTime->ReadyForActivation();
}

void UTimedGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		StopEffect();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTimedGameplayAbility::OnActiveTimeOver()
{
	FinishAbility();
}

void UTimedGameplayAbility::FinishAbility()
{
	constexpr bool bReplicateEndAbility = true;
	constexpr bool bWasCancelled = false;
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), bReplicateEndAbility, bWasCancelled);
}
