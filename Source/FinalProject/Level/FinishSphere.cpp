#include "Level/FinishSphere.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "FinalProjectGameMode.h"
#include "FinalProject.h"

namespace FinishSphereConstants
{
	//Radius of the engine sphere mesh
	constexpr float MeshRadius = 50.f;
}

AFinishSphere::AFinishSphere()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(Radius);
	Collision->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}

	//This material has a "Color" parameter we use to give the finish its own color
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (ShapeMaterial.Succeeded())
	{
		Mesh->SetMaterial(0, ShapeMaterial.Object);
	}
}

void AFinishSphere::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);

	Collision->SetSphereRadius(Radius);
	Mesh->SetRelativeScale3D(FVector(Radius / FinishSphereConstants::MeshRadius));

	if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), SphereColor);
	}
}

void AFinishSphere::NotifyActorBeginOverlap(AActor* otherActor)
{
	Super::NotifyActorBeginOverlap(otherActor);

	APlayerCharacter* Player = Cast<APlayerCharacter>(otherActor);
	if (!Player || bIsFinished)
	{
		return;
	}

	AFinalProjectGameMode* GameMode = GetWorld()->GetAuthGameMode<AFinalProjectGameMode>();
	if (!GameMode)
	{
		UE_LOG(LogFinalProject, Error, TEXT("%s needs the FinalProjectGameMode to finish the game."), *GetName());
		return;
	}

	bIsFinished = true;
	GameMode->FinishGame(Player);
}
