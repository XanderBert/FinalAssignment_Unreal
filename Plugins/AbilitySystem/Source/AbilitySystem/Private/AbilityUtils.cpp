#include "AbilityUtils.h"
#include <Abilities/GameplayAbility.h>
#include <AbilitySystemBlueprintLibrary.h>

void UAbilityUtils::TryCommitAbility(UGameplayAbility* ability, ECommitReturnType& OutputPins)
{
	const bool IsCommited = ability->K2_CommitAbility();

	if (IsCommited)
	{
		OutputPins = ECommitReturnType::OnSucces;
	}
	else 
	{
		OutputPins = ECommitReturnType::OnFailed;
	}
}

float UAbilityUtils::GetAttributeFromOwningActor(UGameplayAbility* ability, FGameplayAttribute attribute, bool& successfullyFoundAttribute)
{
	AActor* owningActor = ability->GetOwningActorFromActorInfo();

	if (!owningActor) 
	{
		successfullyFoundAttribute = false;
		return 0.0f;
	}

	return UAbilitySystemBlueprintLibrary::GetFloatAttribute(owningActor, attribute, successfullyFoundAttribute);
}
