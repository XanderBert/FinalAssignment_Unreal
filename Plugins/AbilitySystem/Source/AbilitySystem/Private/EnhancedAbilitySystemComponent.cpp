#include "EnhancedAbilitySystemComponent.h"
#include "AbilitySystem.h"
#include <EnhancedInputComponent.h>
#include <InputAction.h>
#include <Engine/StreamableManager.h>
#include <Engine/AssetManager.h>
#include <Kismet/GameplayStatics.h>
#include <Components/SkeletalMeshComponent.h>
#include "AbilitySystemAnimInstance.h"

void UEnhancedAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetInputComponent() && AbilityActorInfo.IsValid() && AbilityActorInfo->OwnerActor.IsValid())
	{
		UE_LOG(LogEnhancedAbilitySystem, Log, TEXT("The owning actor: %s, of this component doesn't have an Enhanced Input Component yet. Call BindAbilityInput once it has one."), *AbilityActorInfo->OwnerActor->GetName());
	}

	GrantDefaultAbilitiesAndAttributes();
}

void UEnhancedAbilitySystemComponent::GrantAbility(const TSoftClassPtr<UAbilityDescription> abilityDescription, const FOnAbilityGrantedSignature& onAbilityGranted, int32 level)
{
	//Check if this Ability Is already granted or still being loaded
	if (MappedAbilityClasses.Find(abilityDescription) || PendingAbilityLoads.Contains(abilityDescription))
	{
		return;
	}

	PendingAbilityLoads.Add(abilityDescription);

	//We wrap the component in a WeakObjectPtr so we can check if this is still valid.
	TWeakObjectPtr<UEnhancedAbilitySystemComponent> AbilityComponent(this);

	//Lambda to load asset
	auto LoadAsset = [AbilityComponent, abilityDescription, onAbilityGranted, level]()
	{
		if (!AbilityComponent.IsValid())
		{
			return;
		}

		//The ability got removed while it was loading
		if (AbilityComponent->PendingAbilityLoads.Remove(abilityDescription) == 0)
		{
			return;
		}

		UClass* LoadedClass = abilityDescription.Get();
		if (!IsValid(LoadedClass))
		{
			return;
		}

		if (LoadedClass->IsChildOf(UAbilityDescription::StaticClass()))
		{
			//Use a unique name, a previously removed instance might not be garbage collected yet
			const FName InstanceName = MakeUniqueObjectName(AbilityComponent.Get(), LoadedClass, FName(*LoadedClass->GetName()));
			UAbilityDescription* Instance = NewObject<UAbilityDescription>(AbilityComponent.Get(), LoadedClass, InstanceName);
			if (!IsValid(Instance))
			{
				return;
			}

			//Store new Ability
			AbilityComponent->MappedAbilityClasses.Add(abilityDescription, Instance);

			//Grant our Ability and Bind Input if desired
			if (Instance->AbilityToGrant)
			{
				//Give our Ability
				FGameplayAbilitySpec AbilitySpec(Instance->AbilityToGrant);
				AbilitySpec.RemoveAfterActivation = false;
				AbilitySpec.Level = level;


				//Give the ability an input ID, the input action gets bound to it as soon as we have an input component
				if (Instance->InputAction)
				{
					Instance->BindingInformation.InputID = AbilityInputBindings::GetNextInputID();
					AbilitySpec.InputID = Instance->BindingInformation.InputID;

#if WITH_EDITOR
					//Check if this Input Action is allready registerd for a Ability
					if (AbilityComponent->MappedAbilities.Find(Instance->InputAction))
					{
						UE_LOG(LogEnhancedAbilitySystem, Error, TEXT("InputAction: %s is already used for a ability."), *Instance->InputAction->GetName());
						UE_DEBUG_BREAK();
					}
					else
					{
						AbilityComponent->MappedAbilities.Add(Instance->InputAction).InputID = Instance->BindingInformation.InputID;
					}
#endif
				}


				Instance->AbilitySpecHandle = AbilityComponent->GiveAbility(AbilitySpec);
				AbilityComponent->BindInputForAbility(Instance);
				onAbilityGranted.ExecuteIfBound(Instance->AbilitySpecHandle);
			}


			//Give our Attributes
			for (FAttributeDescription& Attributes : Instance->AttributesToGrant)
			{
				if (!IsValid(Attributes.AttributeSetType))
				{
					continue;
				}

				//First check if we don't have these attributes yet
				if (AbilityComponent->GetAttributeSet(Attributes.AttributeSetType) == nullptr)
				{
					//Spawn The AttributeSet
					AActor* AttributeSetOuter = AbilityComponent->AbilityActorInfo->OwnerActor.Get();
					const FName AttributeSetName = MakeUniqueObjectName(AttributeSetOuter, Attributes.AttributeSetType, FName(*Attributes.AttributeSetType->GetName()));
					UAttributeSet* SpawnedAttributeSet = NewObject<UAttributeSet>(AttributeSetOuter, Attributes.AttributeSetType, AttributeSetName);

					//Initialize the base values when needed
					if (Attributes.InitializationData)
					{
						SpawnedAttributeSet->InitFromMetaDataTable(Attributes.InitializationData);
					}

					//Add the spawned AttributeSet
					AbilityComponent->AddAttributeSetSubobject(SpawnedAttributeSet);
					Instance->SpawnedAttributeSets.Add(SpawnedAttributeSet);
					AbilityComponent->ForceReplication();
				}
				else
				{
					UE_LOG(LogEnhancedAbilitySystem, Error, TEXT("This Attribute Set already Exists"));
					UE_DEBUG_BREAK();
				}
			}

			//Link the Animation layers on Grant
			AbilityComponent->LinkAnimationLayers(Instance->AnimationLinkedOnGrant, Instance->AbilitySpecHandle);
		}
	};


	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(abilityDescription.ToSoftObjectPath(), LoadAsset);
}

void UEnhancedAbilitySystemComponent::RemoveAbility(const TSoftClassPtr<UAbilityDescription> abilityDescription)
{
	//The ability is still loading, cancel the grant
	if (PendingAbilityLoads.Remove(abilityDescription) > 0)
	{
		return;
	}

	//Get the bound Ability
	UAbilityDescription* AbilityToRemove = GetAbilityDescriptionFromClass(abilityDescription);
	if (!IsValid(AbilityToRemove))
	{
		return;
	}

#if WITH_EDITOR
	//If our description defines a input action, and it was registered by this ability
	if (AbilityToRemove->InputAction)
	{
		const FAbilityInputBinding* MappedBinding = MappedAbilities.Find(AbilityToRemove->InputAction);
		if (MappedBinding && MappedBinding->InputID == AbilityToRemove->BindingInformation.InputID)
		{
			MappedAbilities.Remove(AbilityToRemove->InputAction);
		}
	}
#endif


	UnbindInputForAbility(AbilityToRemove);

	//Clear the Ability
	ClearAbility(AbilityToRemove->AbilitySpecHandle);

	//Remove the Attribute sets this ability spawned
	for (UAttributeSet* SpawnedAttributeSet : AbilityToRemove->SpawnedAttributeSets)
	{
		RemoveSpawnedAttribute(SpawnedAttributeSet);
	}

	if (!AbilityToRemove->SpawnedAttributeSets.IsEmpty())
	{
		//Force Replication to make sure the server won't use sets that no longer exist
		ForceReplication();
		AbilityToRemove->SpawnedAttributeSets.Reset();
	}

	//Unlink All Linked Animation Layers
	UnLinkAnimationLayers(AbilityToRemove->AnimationLinkedOnGrant);

	//Mark the Ability for deletion
	AbilityToRemove->MarkAsGarbage();

	MappedAbilityClasses.FindAndRemoveChecked(abilityDescription);
}

void UEnhancedAbilitySystemComponent::BindAbilityInput(UEnhancedInputComponent* inputComponent)
{
	//Remove the bindings from the previous input component
	for (const TPair<TSoftClassPtr<UAbilityDescription>, UAbilityDescription*>& MappedAbility : MappedAbilityClasses)
	{
		UnbindInputForAbility(MappedAbility.Value);
	}

	InputComponent = inputComponent;

	for (const TPair<TSoftClassPtr<UAbilityDescription>, UAbilityDescription*>& MappedAbility : MappedAbilityClasses)
	{
		BindInputForAbility(MappedAbility.Value);
	}
}

void UEnhancedAbilitySystemComponent::LinkAnimationLayers(TArray<TSubclassOf<UAbilitySystemAnimInstance>> animationLayers, FGameplayAbilitySpecHandle abilityHandle)
{
	if (AbilityActorInfo)
	{
		if (AbilityActorInfo->SkeletalMeshComponent.IsValid())
		{
			USkeletalMeshComponent* SkeletalMeshComponent = AbilityActorInfo->SkeletalMeshComponent.Get();

			for (const TSubclassOf<UAbilitySystemAnimInstance>& AnimationLayer : animationLayers)
			{
				//Creates a instance and links them
				SkeletalMeshComponent->LinkAnimClassLayers(AnimationLayer);

				//Get the instance
				UAnimInstance* AnimLayerInstance = SkeletalMeshComponent->GetLinkedAnimLayerInstanceByClass(AnimationLayer);

				UAbilitySystemAnimInstance* MainAnimInstance = Cast<UAbilitySystemAnimInstance>(SkeletalMeshComponent->GetAnimInstance());

				if (UAbilitySystemAnimInstance* AbilitySystemAnimLayerInstance = Cast<UAbilitySystemAnimInstance>(AnimLayerInstance))
				{
					AbilitySystemAnimLayerInstance->InitializeWithAbilitySystem(this, abilityHandle,MainAnimInstance);
				}
			}
		}
	}
}

void UEnhancedAbilitySystemComponent::UnLinkAnimationLayers(TArray<TSubclassOf<UAbilitySystemAnimInstance>> animationLayers)
{
	if (AbilityActorInfo)
	{
		if (AbilityActorInfo->SkeletalMeshComponent.IsValid())
		{
			USkeletalMeshComponent* SkeletalMeshComponent = AbilityActorInfo->SkeletalMeshComponent.Get();

			for (const TSubclassOf<UAbilitySystemAnimInstance>& AnimationLayer : animationLayers)
			{
				UAbilitySystemAnimInstance* AbilitySystemAnimInstance =
				Cast<UAbilitySystemAnimInstance>(SkeletalMeshComponent->GetLinkedAnimLayerInstanceByClass(AnimationLayer));
				if (AbilitySystemAnimInstance && AbilitySystemAnimInstance->bShouldHandleUnlinkingAnimLayer == false )
				{
					AbilitySystemAnimInstance->UnlinkFromInstances(SkeletalMeshComponent);
				}
			}
		}
	}
}

void UEnhancedAbilitySystemComponent::SetLevelForAbility(TSoftClassPtr<UAbilityDescription> abilityDescription, const FOnAbilityGrantedSignature& onLeveledUp, int32 newLevel, bool interuptAbilityIfBusy)
{
	if (interuptAbilityIfBusy)
	{
		RemoveAbility(abilityDescription);
		GrantAbility(abilityDescription, onLeveledUp, newLevel);
		return;
	}

	//Get the Abilityspec of the ability
	FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecFromClass(abilityDescription);
	if (!AbilitySpec)
	{
		return;
	}

	AbilitySpec->Level = newLevel;

	//mark as dirty -> will be leveld up if the ability is not executed.
	MarkAbilitySpecDirty(*AbilitySpec);
	onLeveledUp.ExecuteIfBound(AbilitySpec->Handle);	//TODO: This is not actually the moment where it gets leveled up.
														//Figure out a way how we can track when the Ability System updates the abilitySpec of the ability -> That is the moment where it will be leveledup
}


void UEnhancedAbilitySystemComponent::LevelUpAbility(const TSoftClassPtr<UAbilityDescription>& abilityDescription, const FOnAbilityGrantedSignature& onLeveledUp, bool interuptAbilityIfBusy)
{
	//Get the spec handle of the ability
	FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecFromClass(abilityDescription);
	if (!AbilitySpec)
	{
		return;
	}

	SetLevelForAbility(abilityDescription, onLeveledUp, (AbilitySpec->Level + 1), interuptAbilityIfBusy);
}


void UEnhancedAbilitySystemComponent::GrantDefaultAbilitiesAndAttributes()
{
	for (const TSoftClassPtr<UAbilityDescription>& ability : DefaultAbilities)
	{
		GrantAbility(ability, {});
	}
}


UAbilityDescription* UEnhancedAbilitySystemComponent::GetAbilityDescriptionFromClass(const TSoftClassPtr<UAbilityDescription>& abilityDescription)
{
	UAbilityDescription* Ability = MappedAbilityClasses.FindRef(abilityDescription);

	if (!IsValid(Ability))
	{
		UE_LOG(LogEnhancedAbilitySystem, Error, TEXT("The instanced AbilityDescription: %s, is not valid."), *abilityDescription.GetAssetName());
		return nullptr;
	}

	return Ability;
}

FGameplayAbilitySpec* UEnhancedAbilitySystemComponent::GetAbilitySpecFromClass(const TSoftClassPtr<UAbilityDescription>& abilityDescription)
{
	//Get the spec handle of the ability
	UAbilityDescription* AbilityToLevelUp = GetAbilityDescriptionFromClass(abilityDescription);
	if (!IsValid(AbilityToLevelUp))
	{
		return nullptr;
	}

	FGameplayAbilitySpecHandle SpecHandle = AbilityToLevelUp->AbilitySpecHandle;

	//Update the level
	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
	if (!AbilitySpec)
	{
		UE_LOG(LogEnhancedAbilitySystem, Error, TEXT("The Ability from Handle: %s is not valid."), *SpecHandle.ToString());
		return nullptr;
	}


	return AbilitySpec;
}

UEnhancedInputComponent* UEnhancedAbilitySystemComponent::GetInputComponent()
{
	if (!IsValid(InputComponent) && AbilityActorInfo.IsValid())
	{
		//The avatar is usually the pawn that receives the input, the owner can be something like the player state
		AActor* InputActor = AbilityActorInfo->AvatarActor.IsValid() ? AbilityActorInfo->AvatarActor.Get() : AbilityActorInfo->OwnerActor.Get();
		if (InputActor)
		{
			InputComponent = Cast<UEnhancedInputComponent>(InputActor->InputComponent);
		}
	}

	return InputComponent;
}

void UEnhancedAbilitySystemComponent::BindInputForAbility(UAbilityDescription* abilityDescription)
{
	if (!IsValid(abilityDescription) || !abilityDescription->InputAction || abilityDescription->BindingInformation.bIsBound)
	{
		return;
	}

	//Not having an input component is fine (e.g. AI or the server), BindAbilityInput will bind it once there is one
	UEnhancedInputComponent* EnhancedInputComponent = GetInputComponent();
	if (!EnhancedInputComponent)
	{
		return;
	}

	FAbilityInputBinding& BindingInformation = abilityDescription->BindingInformation;
	BindingInformation.OnPressedHandle = EnhancedInputComponent->BindAction(abilityDescription->InputAction, ETriggerEvent::Started, this, &UEnhancedAbilitySystemComponent::PressInputID, BindingInformation.InputID).GetHandle();
	BindingInformation.OnReleasedHandle = EnhancedInputComponent->BindAction(abilityDescription->InputAction, ETriggerEvent::Completed, this, &UEnhancedAbilitySystemComponent::ReleaseInputID, BindingInformation.InputID).GetHandle();
	BindingInformation.bIsBound = true;
}

void UEnhancedAbilitySystemComponent::UnbindInputForAbility(UAbilityDescription* abilityDescription)
{
	if (!IsValid(abilityDescription) || !abilityDescription->BindingInformation.bIsBound)
	{
		return;
	}

	FAbilityInputBinding& BindingInformation = abilityDescription->BindingInformation;
	if (IsValid(InputComponent))
	{
		InputComponent->RemoveBindingByHandle(BindingInformation.OnPressedHandle);
		InputComponent->RemoveBindingByHandle(BindingInformation.OnReleasedHandle);
	}

	BindingInformation.OnPressedHandle = 0;
	BindingInformation.OnReleasedHandle = 0;
	BindingInformation.bIsBound = false;
}
