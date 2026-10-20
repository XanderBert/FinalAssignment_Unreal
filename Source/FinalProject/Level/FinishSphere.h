#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FinishSphere.generated.h"

class USphereComponent;
class UStaticMeshComponent;

// Sphere at the end of the level. The game is finished once the player touches it.
UCLASS()
class FINALPROJECT_API AFinishSphere : public AActor
{
	GENERATED_BODY()

public:
	AFinishSphere();

	virtual void OnConstruction(const FTransform& transform) override;

protected:
	virtual void NotifyActorBeginOverlap(AActor* otherActor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// Size of the sphere, the collision and the mesh both use it
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Finish", meta = (ClampMin = "1", ForceUnits = "cm"))
	float Radius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Finish")
	FLinearColor SphereColor = FLinearColor::Green;

private:
	bool bIsFinished = false;
};
