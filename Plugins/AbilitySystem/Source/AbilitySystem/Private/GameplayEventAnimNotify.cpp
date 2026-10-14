#include "GameplayEventAnimNotify.h"
#include "AbilitySystemBlueprintLibrary.h"

void UGameplayEventAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), GameplayEventNotify, Payload);
	Super::Notify(MeshComp, Animation, EventReference);
}
