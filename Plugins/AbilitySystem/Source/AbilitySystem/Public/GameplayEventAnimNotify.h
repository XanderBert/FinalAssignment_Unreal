#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayEventAnimNotify.generated.h"

UCLASS(Blueprintable)
class ABILITYSYSTEM_API UGameplayEventAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere) 
	FGameplayTag GameplayEventNotify{};

	UPROPERTY(EditAnywhere)
	FGameplayEventData Payload{};
};
