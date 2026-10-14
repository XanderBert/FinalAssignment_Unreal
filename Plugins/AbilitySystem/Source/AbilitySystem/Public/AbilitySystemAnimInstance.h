#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayAbilitySpec.h"
#include "AbilitySystemAnimInstance.generated.h"

class UEnhancedAbilitySystemComponent;


UCLASS(Abstract, Blueprintable, BlueprintType)
class ABILITYSYSTEM_API UAbilitySystemAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	void InitializeWithAbilitySystem(UEnhancedAbilitySystemComponent* component, FGameplayAbilitySpecHandle abilityHandle,UAbilitySystemAnimInstance* linkedLayerClass = nullptr);

	/*
	* This function can return a nullptr as a linkedLayerClass, make sure to do IsValid checks.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure = False, Category = "Gameplay Ability", meta = (DisplayName = "On Animation Layers Linked"))
	void OnAnimationLayersLinked(UAbilitySystemAnimInstance* linkedLayer = nullptr);

	/*
	* This is the public version of the unlinking function and should be used by outside sources
	e.g. A Gameplay Ability
	* This function will also ALWAYS unlink.
	* It's not intended to use by the anim instance itself, use UnlinkSelf instead.
	*/
	UFUNCTION(BlueprintCallable)
	void UnlinkFromInstances(USkeletalMeshComponent* AbilityActorMesh);


	UFUNCTION(BlueprintNativeEvent, BlueprintPure = False, Category = "Gameplay Ability", meta = (DisplayName = "On Animation Layers Unlinked"))
	void OnAnimationLayersUnlinked(UAbilitySystemAnimInstance* linkedLayer = nullptr);

	UPROPERTY(BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UEnhancedAbilitySystemComponent> AbilityComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Ability System")
	FGameplayAbilitySpecHandle AbilityHandle{};

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool bShouldHandleUnlinkingAnimLayer = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (EditCondition = "bShouldHandleUnlinkingAnimLayer"))
	float BlendOutTime = 0.f;

protected:
	/*
	* This function is intended to be used in blueprints and only to be called by the ABP itself as it checks
	whether this ABP is responsible for unlinking or not.
	*/
	UFUNCTION(BlueprintCallable, meta = (BlueprintProtected))
	void UnlinkSelf(USkeletalMeshComponent* AbilityActorMesh);
private:

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemAnimInstance> LinkedInstance = nullptr;

	virtual void OnAnimationLayersLinked_Implementation(UAbilitySystemAnimInstance* linkedLayer = nullptr) {};


	virtual void OnAnimationLayersUnlinked_Implementation(UAbilitySystemAnimInstance* linkedLayer = nullptr) {};

	void UnlinkInternal(USkeletalMeshComponent* AbilityActorMesh);

};
