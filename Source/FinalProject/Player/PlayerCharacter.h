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

// Third person player that can own one ability at once.
// The input action that activates the owned ability is set on the ability descriptions, the ability system binds it.
UCLASS()
class FINALPROJECT_API APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/**
	* Moves the player to the given transform. Cancels the active ability, but the player keeps owning it.
	* @param respawnTransform where the player should respawn.
	**/
	UFUNCTION(BlueprintCallable, Category = "Player")
	void Respawn(const FTransform& respawnTransform);

protected:
	virtual void PossessedBy(AController* newController) override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* playerInputComponent) override;

	void Move(const FInputActionValue& value);
	void Look(const FInputActionValue& value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FollowCamera;

	// Makes the player visible without a skeletal mesh, hide it once the Mesh component has one
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlaceholderBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UExclusiveAbilitySystemComponent> AbilitySystemComponent;

	// Should also map the input action used by the ability descriptions (e.g. IA_UseAbility)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
};
