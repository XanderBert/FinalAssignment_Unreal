#include "Player/PlayerCharacter.h"
#include "Abilities/ExclusiveAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "UObject/ConstructorHelpers.h"
#include "FinalProject.h"

APlayerCharacter::APlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	//Don't rotate when the controller rotates, that only affects the camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 500.f, 0.f);
	Movement->JumpZVelocity = 500.f;
	Movement->AirControl = 0.35f;
	Movement->MaxWalkSpeed = 500.f;
	Movement->MinAnalogWalkSpeed = 20.f;
	Movement->BrakingDecelerationWalking = 2000.f;
	Movement->BrakingDecelerationFalling = 1500.f;

	//Levitating uses the flying movement mode, make it control the same as walking
	Movement->MaxFlySpeed = Movement->MaxWalkSpeed;
	Movement->BrakingDecelerationFlying = Movement->BrakingDecelerationWalking;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	PlaceholderBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderBody"));
	PlaceholderBody->SetupAttachment(RootComponent);
	PlaceholderBody->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	//The engine cylinder is 100 units in every direction, scale it to the size of the capsule
	PlaceholderBody->SetRelativeScale3D(FVector(0.84f, 0.84f, 1.92f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		PlaceholderBody->SetStaticMesh(CylinderMesh.Object);
	}

	AbilitySystemComponent = CreateDefaultSubobject<UExclusiveAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APlayerCharacter::Respawn(const FTransform& respawnTransform)
{
	//Cancelling reverts the effect of the active ability, like the levitation or the time slowdown
	AbilitySystemComponent->CancelAllAbilities();

	GetCharacterMovement()->StopMovementImmediately();
	TeleportTo(respawnTransform.GetLocation(), respawnTransform.Rotator());

	if (Controller)
	{
		Controller->SetControlRotation(respawnTransform.Rotator());
	}
}

void APlayerCharacter::PossessedBy(AController* newController)
{
	Super::PossessedBy(newController);

	//Refresh the actor info now we have a controller
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void APlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	if (!DefaultMappingContext)
	{
		UE_LOG(LogFinalProject, Error, TEXT("%s has no DefaultMappingContext set, the player can't be controlled."), *GetName());
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
	Super::SetupPlayerInputComponent(playerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(playerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogFinalProject, Error, TEXT("%s needs an Enhanced Input Component, check the Default Input Component Class in the project settings."), *GetName());
		return;
	}

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

	//Binds the input actions of the ability descriptions, also the ones granted later on
	AbilitySystemComponent->BindAbilityInput(EnhancedInputComponent);
}

void APlayerCharacter::Move(const FInputActionValue& value)
{
	if (!Controller)
	{
		UE_LOG(LogFinalProject, Error, TEXT("%s No Controller!."), *GetName());
		return;
	}

	const FVector2D MovementVector = value.Get<FVector2D>();

	//Move relative to where the camera is looking, ignoring the pitch
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void APlayerCharacter::Look(const FInputActionValue& value)
{
	const FVector2D LookAxisVector = value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}
