#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Crusher.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;

// Obstacle that keeps slamming down and rising up again. A player under it while it drops gets crushed and respawns at the player start.
// At normal speed it's open too short to pass under, and it's too high to levitate over.
// It moves on the world time, so the Time slowdown ability slows it down enough to pass under it.
UCLASS()
class FINALPROJECT_API ACrusher : public AActor
{
	GENERATED_BODY()

public:
	ACrusher();

	virtual void OnConstruction(const FTransform& transform) override;
	virtual void Tick(float deltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	// The part that moves up and down, its origin is the bottom of the crusher
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Head;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HeadMesh;

	// Thin zone at the bottom of the head, everything in it gets crushed while dropping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CrushZone;

	// Size of the head. Make it taller than the levitation height, so the player can't levitate over it
	UPROPERTY(EditAnywhere, Category = "Crusher", meta = (ForceUnits = "cm"))
	FVector HeadSize = FVector(400.f, 400.f, 800.f);

	// Height of the bottom of the head when fully raised, it has to be higher than the player to pass under it
	UPROPERTY(EditAnywhere, Category = "Crusher", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RaisedHeight = 250.f;

	UPROPERTY(EditAnywhere, Category = "Crusher", meta = (ClampMin = "1", ForceUnits = "cm/s"))
	float DropSpeed = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Crusher", meta = (ClampMin = "1", ForceUnits = "cm/s"))
	float RiseSpeed = 500.f;

	// How long the crusher stays up before dropping again
	UPROPERTY(EditAnywhere, Category = "Crusher", meta = (ClampMin = "0", ForceUnits = "s"))
	float RaisedWaitTime = 0.3f;

	// How long the crusher stays down before rising again
	UPROPERTY(EditAnywhere, Category = "Crusher", meta = (ClampMin = "0", ForceUnits = "s"))
	float LoweredWaitTime = 0.6f;

	// Extra wait before the first drop, to put multiple crushers out of sync
	UPROPERTY(EditAnywhere, Category = "Crusher", meta = (ClampMin = "0", ForceUnits = "s"))
	float StartDelay = 0.f;

private:
	enum class ECrusherState : uint8
	{
		Raised,
		Dropping,
		Lowered,
		Rising,
	};

	UFUNCTION()
	void OnCrushZoneBeginOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComponent, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

	void SetHeadHeight(float height);
	void CrushOverlappingActors();
	void Crush(AActor* actor);

	ECrusherState State = ECrusherState::Raised;
	float HeadHeight = 0.f;
	float WaitTimeLeft = 0.f;
};
