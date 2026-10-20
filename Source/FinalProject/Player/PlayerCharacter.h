#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UInputAction;
class UExclusiveAbilitySystemComponent;
struct FInputActionValue;

// Third person player shaped like a sphere that can own one ability at once.
// Moves in steps: forward / backward input moves the player by StepDistance, left / right input turns the player by 90 degrees.
// The input action that activates the owned ability is set on the ability descriptions, the ability system binds it.
UCLASS()
class FINALPROJECT_API APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void OnConstruction(const FTransform& transform) override;
	virtual void Tick(float deltaTime) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/**
	* Moves the player to the given transform. Cancels the active ability and the current step or turn, but the player keeps owning the ability.
	* @param respawnTransform where the player should respawn.
	**/
	UFUNCTION(BlueprintCallable, Category = "Player")
	void Respawn(const FTransform& respawnTransform);

protected:
	virtual void PossessedBy(AController* newController) override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* playerInputComponent) override;

	//Only reacts the moment a key gets pressed, holding a key doesn't keep moving the player
	void OnMoveInput(const FInputActionValue& value);
	void OnMoveInputCompleted(const FInputActionValue& value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FollowCamera;

	// The visible sphere, it has the same size as the capsule
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UExclusiveAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FLinearColor BodyColor = FLinearColor::White;

	// How far one step forward or backward moves the player
	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0", ForceUnits = "cm"))
	float StepDistance = 200.f;

	// How fast the player moves during a step
	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "1", ForceUnits = "cm/s"))
	float StepSpeed = 800.f;

	// How fast the player turns by 90 degrees
	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "1", ForceUnits = "deg/s"))
	float TurnSpeed = 360.f;

	// Should also map the input action used by the ability descriptions (e.g. IA_UseAbility)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// 2D axis: Y steps forward (W) or backward (S), X turns clockwise (D) or counterclockwise (A)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

private:
	//Steps in the direction the player is facing, a negative direction steps backward
	void StartStep(float direction);
	//Positive angles turn clockwise when looking from above
	void StartTurn(float angle);
	void StopStepAndTurn();

	bool IsStepping() const { return RemainingStepDistance > 0.f; }
	bool IsTurning() const { return !FMath::IsNearlyZero(RemainingTurnAngle); }

	void TickStep(float deltaTime);
	void TickTurn(float deltaTime);

	FVector2D PreviousMoveInput = FVector2D::ZeroVector;

	FVector StepDirection = FVector::ZeroVector;
	float RemainingStepDistance = 0.f;

	float RemainingTurnAngle = 0.f;
	float TargetYaw = 0.f;
};
