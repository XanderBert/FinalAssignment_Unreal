#include "Obstacles/Crusher.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Player/PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

namespace CrusherConstants
{
	//The crush zone reaches this far below and into the head
	constexpr float CrushZoneHalfHeight = 10.f;

	//Keeps the crush zone a bit inside the sides of the head, so touching the side doesn't crush the player
	constexpr float CrushZoneInset = 5.f;
}

ACrusher::ACrusher()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	Head = CreateDefaultSubobject<USceneComponent>(TEXT("Head"));
	Head->SetupAttachment(SceneRoot);

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(Head);
	HeadMesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		HeadMesh->SetStaticMesh(CubeMesh.Object);
	}

	CrushZone = CreateDefaultSubobject<UBoxComponent>(TEXT("CrushZone"));
	CrushZone->SetupAttachment(Head);
	CrushZone->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
}

void ACrusher::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);

	//The engine cube is 100 units in every direction and centered, put its bottom on the origin of the head
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, HeadSize.Z * 0.5f));
	HeadMesh->SetRelativeScale3D(HeadSize / 100.f);

	CrushZone->SetBoxExtent(FVector(
		FMath::Max(HeadSize.X * 0.5f - CrusherConstants::CrushZoneInset, 1.f),
		FMath::Max(HeadSize.Y * 0.5f - CrusherConstants::CrushZoneInset, 1.f),
		CrusherConstants::CrushZoneHalfHeight));

	//Show the raised position in the editor
	SetHeadHeight(RaisedHeight);
}

void ACrusher::BeginPlay()
{
	Super::BeginPlay();

	CrushZone->OnComponentBeginOverlap.AddDynamic(this, &ACrusher::OnCrushZoneBeginOverlap);

	State = ECrusherState::Raised;
	WaitTimeLeft = RaisedWaitTime + StartDelay;
	SetHeadHeight(RaisedHeight);
}

void ACrusher::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	//deltaTime is scaled by the world time dilation, so the Time slowdown ability slows all of this down
	switch (State)
	{
	case ECrusherState::Raised:
		WaitTimeLeft -= deltaTime;
		if (WaitTimeLeft <= 0.f)
		{
			State = ECrusherState::Dropping;

			//The overlap event only fires for actors that start overlapping, crush the ones that are already in the zone
			CrushOverlappingActors();
		}
		break;

	case ECrusherState::Dropping:
		SetHeadHeight(FMath::Max(HeadHeight - DropSpeed * deltaTime, 0.f));
		if (HeadHeight <= 0.f)
		{
			State = ECrusherState::Lowered;
			WaitTimeLeft = LoweredWaitTime;
		}
		break;

	case ECrusherState::Lowered:
		WaitTimeLeft -= deltaTime;
		if (WaitTimeLeft <= 0.f)
		{
			State = ECrusherState::Rising;
		}
		break;

	case ECrusherState::Rising:
		SetHeadHeight(FMath::Min(HeadHeight + RiseSpeed * deltaTime, RaisedHeight));
		if (HeadHeight >= RaisedHeight)
		{
			State = ECrusherState::Raised;
			WaitTimeLeft = RaisedWaitTime;
		}
		break;
	}
}

void ACrusher::OnCrushZoneBeginOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComponent, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	if (State == ECrusherState::Dropping)
	{
		Crush(otherActor);
	}
}

void ACrusher::SetHeadHeight(float height)
{
	HeadHeight = height;

	//Not swept, the crush zone sits below the head and crushes the player before the head reaches them
	Head->SetRelativeLocation(FVector(0.f, 0.f, HeadHeight));
}

void ACrusher::CrushOverlappingActors()
{
	TArray<AActor*> OverlappingActors;
	CrushZone->GetOverlappingActors(OverlappingActors);

	for (AActor* OverlappingActor : OverlappingActors)
	{
		Crush(OverlappingActor);
	}
}

void ACrusher::Crush(AActor* actor)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(actor);
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (!Player || !GameMode)
	{
		return;
	}

	//The same player start the player spawned at when the level started
	if (AActor* PlayerStart = GameMode->FindPlayerStart(Player->GetController()))
	{
		Player->Respawn(PlayerStart->GetActorTransform());
	}
}
