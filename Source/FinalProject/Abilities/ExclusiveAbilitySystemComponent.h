#pragma once

#include "CoreMinimal.h"
#include "EnhancedAbilitySystemComponent.h"
#include "ExclusiveAbilitySystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOwnedAbilityChangedSignature, TSoftClassPtr<UAbilityDescription>, ownedAbility);

// Ability system component whose owner can only own one ability at once.
// Granting an ability through GrantExclusiveAbility removes the ability that was owned before.
UCLASS(ClassGroup = (AbilitySystem), meta = (BlueprintSpawnableComponent))
class FINALPROJECT_API UExclusiveAbilitySystemComponent : public UEnhancedAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/**
	* Grants the ability and removes the ability that was owned before, ending it if it's still active.
	* @param abilityDescription the description of the ability the owner should own.
	**/
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void GrantExclusiveAbility(UPARAM(meta = (AllowAbstract = false)) const TSoftClassPtr<UAbilityDescription> abilityDescription);

	UFUNCTION(BlueprintPure, Category = "Abilities")
	TSoftClassPtr<UAbilityDescription> GetOwnedAbility() const { return OwnedAbility; }

	// Fires when the owned ability changed.
	UPROPERTY(BlueprintAssignable, Category = "Abilities")
	FOnOwnedAbilityChangedSignature OnOwnedAbilityChanged;

private:
	UPROPERTY(Transient)
	TSoftClassPtr<UAbilityDescription> OwnedAbility{};
};
