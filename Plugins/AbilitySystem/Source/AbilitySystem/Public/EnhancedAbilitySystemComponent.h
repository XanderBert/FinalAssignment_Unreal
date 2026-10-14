#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "EnhancedAbilitySystemComponent.generated.h"

class UInputAction;
class UEnhancedInputComponent;
class UAbilitySystemAnimInstance;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAbilityGrantedSignature, FGameplayAbilitySpecHandle, AbilitySpecHandle);

namespace AbilityInputBindings
{
	constexpr int32 InvalidInputID{};
	inline int32 IncrementingInputID = InvalidInputID;
	inline int32 GetNextInputID() { return ++IncrementingInputID; }
}

USTRUCT()
struct FAttributeDescription
{
	GENERATED_BODY()

	// Ability set to grant
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeSet> AttributeSetType{};

	// Data table reference to initialize the attributes with, if any (can be left unset)
	UPROPERTY(EditAnywhere)
	UDataTable* InitializationData = nullptr;
};


USTRUCT()
struct FAbilityInputBinding
{
	GENERATED_BODY()

	int32 InputID{};
	int32 OnPressedHandle{};
	int32 OnReleasedHandle{};
	bool bIsBound = false;
};


//We can only give it attribute sets or Abilities if desired
//But you must have a valid AbilityToGrant if you pass a InputAction
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, CollapseCategories, NotPlaceable, meta = (DontUseGenericSpawnObject, DataOnly))
class ABILITYSYSTEM_API UAbilityDescription : public UObject
{
	GENERATED_BODY()

public:

	//The Ability We want to grant
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayAbility> AbilityToGrant{};

	//The optional AttributeSet we want to grant.
	UPROPERTY(EditAnywhere)
	TArray<FAttributeDescription> AttributesToGrant{};

	//The optional Input Action We want to bind to activating this ability.
	UPROPERTY(EditAnywhere)
	UInputAction* InputAction{};

	//The animation layers that should be linked on granting of this ability
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UAbilitySystemAnimInstance>> AnimationLinkedOnGrant{};

	//Will contain all the binding ID's of the ability
	FAbilityInputBinding BindingInformation{};

	//Will contain the specHandle of the granted ability
	FGameplayAbilitySpecHandle AbilitySpecHandle{};

	//The attribute sets this ability spawned, so removing the ability only removes these
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAttributeSet>> SpawnedAttributeSets{};
};

// This class Describes our full ability,
// Which input action it should bind on,
// what animation layers it should link,
// what attributes it gets.
// See UAbilityDescription
UCLASS(BlueprintType, Blueprintable, EditInlineNew, CollapseCategories, NotPlaceable, meta = (DontUseGenericSpawnObject, DataOnly))
class ABILITYSYSTEM_API UAbilityDescriptionBlueprint : public UBlueprint
{
	GENERATED_BODY()
public:
	UAbilityDescriptionBlueprint(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
	}

#if WITH_EDITOR
	virtual bool SupportedByDefaultBlueprintFactory() const override
	{
		return false;
	}
#endif
};


UCLASS( ClassGroup=(AbilitySystem), HideCategories=(AttributeTest,Sockets, Navigation), meta = (BlueprintSpawnableComponent))
class ABILITYSYSTEM_API UEnhancedAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	/**
	* @param abilityDescription the description of the whole ability you want to grant.
	* @param onAbilityGranted passed in delegate that fires if the ability is granted
	* @param level the starting level of the the granted ability
	**/
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void GrantAbility(UPARAM(meta = (AllowAbstract = false)) const TSoftClassPtr<UAbilityDescription> abilityDescription, const FOnAbilityGrantedSignature& onAbilityGranted, int32 level = 1);


	/**
	* @param abilityDescription the description of the ability you want to remove.
	**/
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void RemoveAbility(UPARAM(meta = (AllowAbstract = false)) const TSoftClassPtr<UAbilityDescription> abilityDescription);


	/**
	* Binds the input actions of all granted abilities on the given input component.
	* Call this once the owner has its input component (e.g. from SetupPlayerInputComponent),
	* abilities granted before the input component existed get their input bound here.
	* @param inputComponent the enhanced input component to bind the ability input actions on.
	**/
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void BindAbilityInput(UEnhancedInputComponent* inputComponent);


	/**
	*
	* @param animationLayers the list of animation instances that should be linked on our skeleton mesh of the owner.
	* @param abilityHandle the handle to the ability that uses this animation. The default constructed handle will have as value INDEX_NONE, This way we can keep it optional to pass a abilityHandle
	**/
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void LinkAnimationLayers(TArray<TSubclassOf<UAbilitySystemAnimInstance>> animationLayers, FGameplayAbilitySpecHandle abilityHandle = FGameplayAbilitySpecHandle());


	/**
	* @param animationLayers the list of animation instances that should be linked on our skeleton mesh of the owner.
	**/
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void UnLinkAnimationLayers(TArray<TSubclassOf<UAbilitySystemAnimInstance>> animationLayers);



	/**
	* @param abilityDescription the description of the ability you want to level up.
	* @param onLeveledUp the delegate that will fire once the ability is leveled up.
	* @param newLevel the new level of the ability.
	* @param interuptAbilityIfBusy if true the ability will be instantly removed and added again with the new level. Otherwise the server will level up the ability if its not busy.
	* Please Note if you interupt your ability while leveling up. The AttributeSets will be reset to their default value.
	**/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Abilities")
	void SetLevelForAbility(UPARAM(meta = (AllowAbstract = false)) const TSoftClassPtr<UAbilityDescription> abilityDescription, const FOnAbilityGrantedSignature& onLeveledUp, int32 newLevel = 1, bool interuptAbilityIfBusy = false);


	/**
	* @param abilityDescription the description of the ability you want to level up.
	* @param onLeveledUp the delegate that will fire once the ability is leveled up.
	* @param interuptAbilityIfBusy if true the ability will be instantly removed and added again with the new level. Otherwise the server will level up the ability if its not busy.
	**/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Abilities")
	void LevelUpAbility(UPARAM(meta = (AllowAbstract = false)) const TSoftClassPtr<UAbilityDescription>& abilityDescription, const FOnAbilityGrantedSignature& onLeveledUp, bool interuptAbilityIfBusy = false);



	// An array holding the default abilities / attributs / input
	UPROPERTY(EditDefaultsOnly, Category = Abilities)
	TArray<TSoftClassPtr<UAbilityDescription>> DefaultAbilities{};

protected:
	void GrantDefaultAbilitiesAndAttributes();

private:

	UAbilityDescription* GetAbilityDescriptionFromClass(const TSoftClassPtr<UAbilityDescription>& abilityDescription);
	FGameplayAbilitySpec* GetAbilitySpecFromClass(const TSoftClassPtr<UAbilityDescription>& abilityDescription);

	//Returns the cached input component, or looks it up on the avatar / owner if we don't have one yet
	UEnhancedInputComponent* GetInputComponent();
	void BindInputForAbility(UAbilityDescription* abilityDescription);
	void UnbindInputForAbility(UAbilityDescription* abilityDescription);


#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	TMap<UInputAction*, FAbilityInputBinding> MappedAbilities{};
#endif

	UPROPERTY(Transient)
	TMap<TSoftClassPtr<UAbilityDescription>, UAbilityDescription*> MappedAbilityClasses{};

	//Abilities that are still being async loaded, so they don't get granted twice
	TSet<TSoftClassPtr<UAbilityDescription>> PendingAbilityLoads{};

	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputComponent> InputComponent = nullptr;
};
