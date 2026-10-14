#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AbilityUtils.generated.h"



UENUM(BlueprintType)
enum class ECommitReturnType : uint8
{
	OnSucces,
	OnFailed,
};

UCLASS()
class ABILITYSYSTEM_API UAbilityUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	
	/**
	* Tries to commit the ability.
	**/
	UFUNCTION(BlueprintCallable, Category = "Ability System", meta = (ExpandEnumAsExecs = "OutputPins", DefaultToSelf = "ability"))
	static void TryCommitAbility(UGameplayAbility* ability, ECommitReturnType& OutputPins);


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability System", meta = (DefaultToSelf = "ability"))
	static float GetAttributeFromOwningActor(UGameplayAbility* ability,FGameplayAttribute attribute, bool& successfullyFoundAttribute);

};
