#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilityPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UAbilityDescription;

// Sphere that represents an ability. The player owns the ability once they touch the sphere, and the sphere disappears.
// Owning this ability makes the player lose the ability they owned before.
UCLASS()
class FINALPROJECT_API AAbilityPickup : public AActor
{
	GENERATED_BODY()

public:
	AAbilityPickup();

	virtual void OnConstruction(const FTransform& transform) override;

protected:
	virtual void BeginPlay() override;
	virtual void NotifyActorBeginOverlap(AActor* otherActor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// The ability the player gets when touching this sphere
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowAbstract = false))
	TSoftClassPtr<UAbilityDescription> AbilityDescription{};

	// Every ability has its own color, Levitation is orange and Time slowdown is blue
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FLinearColor SphereColor = FLinearColor::White;
};
