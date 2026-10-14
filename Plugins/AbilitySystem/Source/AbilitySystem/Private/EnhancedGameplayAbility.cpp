#include "EnhancedGameplayAbility.h"
#include "AbilitySystem.h"
#include "AbilitySystemAnimInstance.h"
#include "EnhancedAbilitySystemComponent.h"
#include <Components/SkeletalMeshComponent.h>
#include <TimerManager.h>

void UEnhancedGameplayAbility::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	if (!ActorInfo)
	{
		UE_LOG(LogEnhancedAbilitySystem, Error, TEXT("UEnhancedGameplayAbility::PreActivate ActorInfo is null"));
		return;
	}

	//Link the Anim Layers
	if (ActorInfo->SkeletalMeshComponent.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		if (UEnhancedAbilitySystemComponent* AbilityComponent = Cast<UEnhancedAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
		{
			USkeletalMeshComponent* SkeletalMeshComponent = ActorInfo->SkeletalMeshComponent.Get();

			for (const TSubclassOf<UAbilitySystemAnimInstance>& AnimationLayer : AnimationLayersLinkedOnActivation)
			{
				//Creates a instance and links them
				SkeletalMeshComponent->LinkAnimClassLayers(AnimationLayer);

				//Get the instance
				UAnimInstance* AnimLayerInstance = SkeletalMeshComponent->GetLinkedAnimLayerInstanceByClass(AnimationLayer);

				UAbilitySystemAnimInstance* MainAnimInstance = Cast<UAbilitySystemAnimInstance>(SkeletalMeshComponent->GetAnimInstance());

				if (UAbilitySystemAnimInstance* AbilitySystemAnimLayerInstance = Cast<UAbilitySystemAnimInstance>(AnimLayerInstance))
				{
					AbilitySystemAnimLayerInstance->InitializeWithAbilitySystem(AbilityComponent, Handle, MainAnimInstance);
				}
			}
		}
	}
	EndAbilityTimerDelegate.BindUFunction(this, FName("PrepareEndAbility"), TWeakObjectPtr<UEnhancedGameplayAbility>(this));
}

void UEnhancedGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(EndAbilityTimer);
		}

		UnlinkAnimationLayers(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UEnhancedGameplayAbility::CancelAbility(bool forceUnlinkAnimLayers)
{
	UnlinkAnimationLayers(forceUnlinkAnimLayers);
	Super::K2_CancelAbility();
}

void UEnhancedGameplayAbility::UnlinkAnimationLayers(bool force)
{
	FGameplayAbilityActorInfo AbilityActorInfo = GetActorInfo();
	if (AbilityActorInfo.SkeletalMeshComponent.IsValid())
	{
		USkeletalMeshComponent* SkeletalMeshComponent = AbilityActorInfo.SkeletalMeshComponent.Get();

		// unlink anim layers that are linked on activation
		for (const TSubclassOf<UAbilitySystemAnimInstance>& AnimationLayer : AnimationLayersLinkedOnActivation)
		{
			UAbilitySystemAnimInstance* AbilitySystemAnimInstance =
				Cast<UAbilitySystemAnimInstance>(SkeletalMeshComponent->GetLinkedAnimLayerInstanceByClass(AnimationLayer));
			if (AbilitySystemAnimInstance &&
				(AbilitySystemAnimInstance->bShouldHandleUnlinkingAnimLayer == false || force))
			{
				AbilitySystemAnimInstance->UnlinkFromInstances(SkeletalMeshComponent);
			}
		}
		// unlink anim layers that are added at runtime
		for (const TSubclassOf<UAbilitySystemAnimInstance>& AnimationLayer : AnimationLayersAddedAtRuntime)
		{
			UAbilitySystemAnimInstance* AbilitySystemAnimInstance =
				Cast<UAbilitySystemAnimInstance>(SkeletalMeshComponent->GetLinkedAnimLayerInstanceByClass(AnimationLayer));
			if (AbilitySystemAnimInstance &&
				(AbilitySystemAnimInstance->bShouldHandleUnlinkingAnimLayer == false || force))
			{
				AbilitySystemAnimInstance->UnlinkFromInstances(SkeletalMeshComponent);
			}
		}
	}
}


void UEnhancedGameplayAbility::InitializeLayerWithAbilitySystem(TSubclassOf<UAbilitySystemAnimInstance> animClass)
{
	FGameplayAbilityActorInfo AbilityActorInfo = GetActorInfo();
	if (AbilityActorInfo.SkeletalMeshComponent.IsValid())
	{
		USkeletalMeshComponent* SkeletalMeshComponent = AbilityActorInfo.SkeletalMeshComponent.Get();
		//Creates a instance and links them
		SkeletalMeshComponent->LinkAnimClassLayers(animClass);


		UAbilitySystemAnimInstance* AbilityAnimInstance =
			Cast<UAbilitySystemAnimInstance>(SkeletalMeshComponent->GetLinkedAnimLayerInstanceByClass(animClass));

		UAbilitySystemAnimInstance* MainAnimInstance = Cast<UAbilitySystemAnimInstance>(SkeletalMeshComponent->GetAnimInstance());

		if (AbilityAnimInstance)
		{
			UEnhancedAbilitySystemComponent* AbilityComponent = Cast<UEnhancedAbilitySystemComponent>(AbilityActorInfo.AbilitySystemComponent.Get());
			AbilityAnimInstance->InitializeWithAbilitySystem(AbilityComponent, CurrentSpecHandle, MainAnimInstance);
			AnimationLayersAddedAtRuntime.AddUnique(animClass);
		}
	}
}

void UEnhancedGameplayAbility::BlendOutAndEndAbility(float BlendOutTime)
{
	GetWorld()->GetTimerManager().SetTimer(EndAbilityTimer, EndAbilityTimerDelegate, BlendOutTime, false);
}

void UEnhancedGameplayAbility::PrepareEndAbility(const TWeakObjectPtr<UEnhancedGameplayAbility>& weakThis)
{
	if (weakThis.IsValid())
	{
		weakThis->K2_EndAbility();
	}
}

void UEnhancedGameplayAbility::Tick(float deltaTime)
{
	if (LastFrameNumberWeTicked == GFrameCounter)
	{
		checkNoEntry();
		return;
	}
	LastFrameNumberWeTicked = GFrameCounter;

	TickLogic(deltaTime);
}
