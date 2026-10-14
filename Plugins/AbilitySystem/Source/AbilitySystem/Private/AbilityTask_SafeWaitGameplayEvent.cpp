#include "AbilityTask_SafeWaitGameplayEvent.h"
#include "AbilitySystem.h"
#include "Abilities/GameplayAbility.h"
#include <TimerManager.h>


UAbilityTask_SafeWaitGameplayEvent::UAbilityTask_SafeWaitGameplayEvent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

UAbilityTask_SafeWaitGameplayEvent* UAbilityTask_SafeWaitGameplayEvent::SafeWaitGameplayEvent(UGameplayAbility* owningAbility, FGameplayTag eventTag, AActor* optionalTarget, bool onlyTriggerOnce, bool onlyMatchExact, float maxWaitTime, bool breakOnTimeLimit)
{
	UAbilityTask_SafeWaitGameplayEvent* MyObj = NewAbilityTask<UAbilityTask_SafeWaitGameplayEvent>(owningAbility);
	MyObj->Tag = eventTag;
	MyObj->SetExternalTarget(optionalTarget);
	MyObj->OnlyTriggerOnce = onlyTriggerOnce;
	MyObj->OnlyMatchExact = onlyMatchExact;
	MyObj->OwningActor = owningAbility->GetOwningActorFromActorInfo();
	MyObj->MaxWaitTime = maxWaitTime;
	MyObj->BreakOnTimeLimit= breakOnTimeLimit;

	return MyObj;
}

void UAbilityTask_SafeWaitGameplayEvent::Activate()
{
	Super::Activate();
	StartTimer();
}

void UAbilityTask_SafeWaitGameplayEvent::GameplayEventContainerCallback(FGameplayTag MatchingTag, const FGameplayEventData* Payload)
{
	ClearTimer();
	Super::GameplayEventContainerCallback(MatchingTag, Payload);
}

void UAbilityTask_SafeWaitGameplayEvent::OnDestroy(bool AbilityEnding)
{
	ClearTimer();
	Super::OnDestroy(AbilityEnding);
}

void UAbilityTask_SafeWaitGameplayEvent::SafeTimerCallback()
{
	UE_LOG(LogEnhancedAbilitySystem, Warning, TEXT("Wait For Gameplay Event: %s for actor: %s has reached time limit."), *Tag.GetTagName().ToString(), *GetNameSafe(OwningActor.Get()));
	if (BreakOnTimeLimit)
	{
		UE_DEBUG_BREAK();
	}

	//Act as if the event was received so the ability doesn't get stuck.
	//This only triggers this task, other listeners for the tag don't receive a fake event.
	FGameplayEventData Payload;
	Payload.EventTag = Tag;
	GameplayEventContainerCallback(Tag, &Payload);
}

void UAbilityTask_SafeWaitGameplayEvent::StartTimer()
{
	if (MaxWaitTime >= FLT_EPSILON)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(TimerHandle, this, &UAbilityTask_SafeWaitGameplayEvent::SafeTimerCallback, MaxWaitTime, false);
		}
	}
}

void UAbilityTask_SafeWaitGameplayEvent::ClearTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
}
