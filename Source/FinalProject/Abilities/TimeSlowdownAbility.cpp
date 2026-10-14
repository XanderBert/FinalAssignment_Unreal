#include "Abilities/TimeSlowdownAbility.h"
#include "GameFramework/WorldSettings.h"

bool UTimeSlowdownAbility::StartEffect()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	AWorldSettings* WorldSettings = World ? World->GetWorldSettings() : nullptr;
	if (!Avatar || !WorldSettings)
	{
		return false;
	}

	PreviousWorldTimeDilation = WorldSettings->TimeDilation;
	PreviousAvatarTimeDilation = Avatar->CustomTimeDilation;

	//SetTimeDilation clamps the value, so continue with the value that was actually applied
	AppliedWorldTimeDilation = WorldSettings->SetTimeDilation(WorldTimeDilation);

	//Speed the player up by the same amount, so only the world around the player slows down
	Avatar->CustomTimeDilation = PreviousAvatarTimeDilation / AppliedWorldTimeDilation;

	bIsTimeSlowedDown = true;
	return true;
}

void UTimeSlowdownAbility::StopEffect()
{
	if (!bIsTimeSlowedDown)
	{
		return;
	}
	bIsTimeSlowedDown = false;

	UWorld* World = GetWorld();
	if (AWorldSettings* WorldSettings = World ? World->GetWorldSettings() : nullptr)
	{
		WorldSettings->SetTimeDilation(PreviousWorldTimeDilation);
	}

	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		Avatar->CustomTimeDilation = PreviousAvatarTimeDilation;
	}
}

float UTimeSlowdownAbility::GetActiveTimeInWorldTime() const
{
	return ActiveTime * AppliedWorldTimeDilation;
}
