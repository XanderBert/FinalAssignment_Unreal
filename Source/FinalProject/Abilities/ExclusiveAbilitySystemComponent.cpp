#include "Abilities/ExclusiveAbilitySystemComponent.h"
#include "AssertsLibrary.h"

void UExclusiveAbilitySystemComponent::GrantExclusiveAbility(const TSoftClassPtr<UAbilityDescription> abilityDescription)
{
	UAssertsLibrary::BlueprintBreak(!abilityDescription.IsNull(), TEXT("GrantExclusiveAbility was called without an ability description."));
	if (abilityDescription.IsNull() || abilityDescription == OwnedAbility)
	{
		return;
	}

	//Remove the previous ability first, this also ends it when it's still active.
	//RemoveAbility is instant while GrantAbility loads async, so both abilities can use the same input action.
	if (!OwnedAbility.IsNull())
	{
		RemoveAbility(OwnedAbility);
	}

	OwnedAbility = abilityDescription;
	GrantAbility(OwnedAbility, FOnAbilityGrantedSignature());

	OnOwnedAbilityChanged.Broadcast(OwnedAbility);
}
