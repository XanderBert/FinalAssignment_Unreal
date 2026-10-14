#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Tickable.h"
#include "EnhancedGameplayAbility.generated.h"

class UAbilitySystemAnimInstance;


UCLASS(BlueprintType, Blueprintable)
class ABILITYSYSTEM_API UEnhancedGameplayAbility : public UGameplayAbility, public FTickableGameObject
{
	GENERATED_BODY()
public:
	//CancelAbility(bool) below would otherwise hide the engine's CancelAbility overloads
	using UGameplayAbility::CancelAbility;

protected:

	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData = nullptr) override;

	//Unlinks the animation layers that don't handle their own unlinking
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/*
	This function links an ABP to the Actor info's skeletal mesh.
	It's meant to be used when you specifically want to control the linking,
	so not on Grant/Activation
	*/
	UFUNCTION(BlueprintCallable)
	void InitializeLayerWithAbilitySystem(TSubclassOf<UAbilitySystemAnimInstance> animClass);

	/*
	* This function does not fire End ability this frame, instead it receives a time until it fires.
	* This is supposed to be used in tandem with blend out times of animation blueprints.
	*/
	UFUNCTION(BlueprintCallable)
	void BlendOutAndEndAbility(float BlendOutTime);

	//The animation layers that should be linked on activation of this ability
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UAbilitySystemAnimInstance>> AnimationLayersLinkedOnActivation{};

	//The animation layers that are added by InitializeLayerWithAbilitySystem
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UAbilitySystemAnimInstance>> AnimationLayersAddedAtRuntime{};

	UFUNCTION(BlueprintCallable)
	void CancelAbility(bool forceUnlinkAnimLayers);



#pragma region Tickable
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = False, Category = "Gameplay Ability")
	bool CanTick() const;
	virtual bool CanTick_Implementation() const { return bCanTick; };

	UFUNCTION(BlueprintNativeEvent, BlueprintPure = False, Category = "Gameplay Ability", meta = (DisplayName = "Tick"))
	void TickLogic(float deltaTime);
	virtual void TickLogic_Implementation(float deltaTime) {};



	// If false, CanTick won't ever be test for tickability of this stage logic
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Config")
	bool bCanTick = false;
#pragma endregion

private:

	void UnlinkAnimationLayers(bool force);

	UFUNCTION()
	//pass in weakThis as a safety should the "EndAbility" be called before the
	// blend out happens
	void PrepareEndAbility(const TWeakObjectPtr<UEnhancedGameplayAbility>& weakThis);
	FTimerHandle EndAbilityTimer;
	FTimerDelegate EndAbilityTimerDelegate;

#pragma region Tickable
	virtual void Tick(float deltaTime) override;

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Conditional;
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UEnhancedGameplayAbility, STATGROUP_Tickables);
	}

	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual bool IsTickable() const override { return bCanTick && IsValid(this) && !IsUnreachable() && CanTick(); }
	virtual bool IsAllowedToTick() const override { return !HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject); }
	// The last frame number we were ticked.
	// Guard because we don't want to tick multiple times per frame
	uint32 LastFrameNumberWeTicked = INDEX_NONE;
#pragma endregion
};
