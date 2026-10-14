#include "Abilities/AbilityPickup.h"
#include "Abilities/ExclusiveAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "FinalProject.h"

AAbilityPickup::AAbilityPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(50.f);
	Collision->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	//The engine sphere has a radius of 50, the same as the collision
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}

	//This material has a "Color" parameter we use to give every ability its own color
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (ShapeMaterial.Succeeded())
	{
		Mesh->SetMaterial(0, ShapeMaterial.Object);
	}
}

void AAbilityPickup::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);

	if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), SphereColor);
	}
}

void AAbilityPickup::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(!AbilityDescription.IsNull(), TEXT("Ability pickup %s has no AbilityDescription set, touching it does nothing."), *GetName());
}

void AAbilityPickup::NotifyActorBeginOverlap(AActor* otherActor)
{
	Super::NotifyActorBeginOverlap(otherActor);

	UExclusiveAbilitySystemComponent* AbilitySystem = Cast<UExclusiveAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(otherActor));
	if (!AbilitySystem || AbilityDescription.IsNull())
	{
		return;
	}

	AbilitySystem->GrantExclusiveAbility(AbilityDescription);
	Destroy();
}
