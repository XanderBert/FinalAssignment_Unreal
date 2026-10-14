#include "AbilitySystemAnimInstance.h"
#include "EnhancedAbilitySystemComponent.h"
#include <Components/SkeletalMeshComponent.h>



void UAbilitySystemAnimInstance::InitializeWithAbilitySystem(UEnhancedAbilitySystemComponent* component, FGameplayAbilitySpecHandle abilityHandle, UAbilitySystemAnimInstance* linkedLayer)
{
	AbilityComponent = component;
	AbilityHandle = abilityHandle;

	if (linkedLayer && linkedLayer != this)
	{
		LinkedInstance = linkedLayer;
		this->OnAnimationLayersLinked(LinkedInstance);
		// We want the same event to call on the class we're linking with
		LinkedInstance->OnAnimationLayersLinked(this);
	}
}

void UAbilitySystemAnimInstance::UnlinkFromInstances(USkeletalMeshComponent* AbilityActorMesh)
{
	if (bShouldHandleUnlinkingAnimLayer)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("This anim instance is responsible to unlink itself, but we're unlinking anyway"));
	}
	UnlinkInternal(AbilityActorMesh);

}

void UAbilitySystemAnimInstance::UnlinkSelf(USkeletalMeshComponent* AbilityActorMesh)
{
	checkf(bShouldHandleUnlinkingAnimLayer, TEXT("Anim instance is handling unlinking, but bShouldHandleUnlinkingAnimLayer is false."));
	UnlinkInternal(AbilityActorMesh);
}

void UAbilitySystemAnimInstance::UnlinkInternal(USkeletalMeshComponent* AbilityActorMesh)
{
	if (!AbilityActorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to unlink %s but the mesh is not valid"), *GetName());
		return;
	}

	//The main anim instance is only set when it's also a UAbilitySystemAnimInstance, the layer still needs unlinking without it
	if (LinkedInstance)
	{
		this->OnAnimationLayersUnlinked(LinkedInstance);
		LinkedInstance->OnAnimationLayersUnlinked(this);
		LinkedInstance = nullptr;
	}

	AbilityActorMesh->UnlinkAnimClassLayers(this->GetClass());
}

