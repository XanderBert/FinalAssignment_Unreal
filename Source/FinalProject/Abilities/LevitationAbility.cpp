#include "Abilities/LevitationAbility.h"
#include "GameFramework/CharacterMovementComponent.h"

ULevitationAbility::ULevitationAbility()
{
	//CanTick decides when we actually tick, only while moving up or down
	bCanTick = true;
}

bool ULevitationAbility::StartEffect()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement || !Movement->UpdatedComponent)
	{
		return false;
	}

	OriginalHeight = Movement->UpdatedComponent->GetComponentLocation().Z;

	//Flying ignores gravity, so the player stays at the levitation height but can still move around
	Movement->SetMovementMode(MOVE_Flying);
	Movement->Velocity.Z = 0.f;

	Phase = ELevitationPhase::Rising;
	return true;
}

void ULevitationAbility::StopEffect()
{
	Phase = ELevitationPhase::None;

	//Let gravity take over again, the movement component switches back to walking once the player lands.
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Movement && Movement->MovementMode == MOVE_Flying)
	{
		Movement->SetMovementMode(MOVE_Falling);
	}
}

void ULevitationAbility::OnActiveTimeOver()
{
	Phase = ELevitationPhase::Lowering;
}

bool ULevitationAbility::CanTick_Implementation() const
{
	return Phase == ELevitationPhase::Rising || Phase == ELevitationPhase::Lowering;
}

void ULevitationAbility::TickLogic_Implementation(float deltaTime)
{
	//Moving the player can trigger overlaps that end this ability
	//so check the phase is still the same after moving
	switch (Phase)
	{
	case ELevitationPhase::Rising:
		if (MoveTowardsHeight(OriginalHeight + LevitationHeight, deltaTime) && Phase == ELevitationPhase::Rising)
		{
			Phase = ELevitationPhase::Hovering;
		}
		break;

	case ELevitationPhase::Lowering:
		if (MoveTowardsHeight(OriginalHeight, deltaTime) && Phase == ELevitationPhase::Lowering)
		{
			FinishAbility();
		}
		break;

	default:
		break;
	}
}

bool ULevitationAbility::MoveTowardsHeight(float targetHeight, float deltaTime)
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement || !Movement->UpdatedComponent)
	{
		return true;
	}

	const float MaxDistance = VerticalSpeed * deltaTime;
	const float CurrentHeight = Movement->UpdatedComponent->GetComponentLocation().Z;
	const float Distance = FMath::Clamp(targetHeight - CurrentHeight, -MaxDistance, MaxDistance);

	//Sweep so the player can't levitate through ceilings, and lands on top of the Step when lowering above it
	FHitResult Hit;
	Movement->SafeMoveUpdatedComponent(FVector(0.f, 0.f, Distance), Movement->UpdatedComponent->GetComponentQuat(), true, Hit);

	const float NewHeight = Movement->UpdatedComponent->GetComponentLocation().Z;
	return Hit.bBlockingHit || FMath::IsNearlyEqual(NewHeight, targetHeight, 0.1f);
}

UCharacterMovementComponent* ULevitationAbility::GetCharacterMovement() const
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	return ActorInfo ? Cast<UCharacterMovementComponent>(ActorInfo->MovementComponent.Get()) : nullptr;
}
