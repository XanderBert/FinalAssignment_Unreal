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
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "FinalProject.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(50.f, 50.f);

	//Don't rotate when the controller rotates, that only affects the camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	//The player only turns through the turn input, stepping backward shouldn't make the player face backward
	Movement->bOrientRotationToMovement = false;
	Movement->MaxWalkSpeed = 500.f;
	Movement->MinAnalogWalkSpeed = 20.f;
	Movement->BrakingDecelerationWalking = 2000.f;
	Movement->BrakingDecelerationFalling = 1500.f;

	//Levitating uses the flying movement mode, make it control the same as walking
	Movement->MaxFlySpeed = Movement->MaxWalkSpeed;
	Movement->BrakingDecelerationFlying = Movement->BrakingDecelerationWalking;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.f;
	//Follow the rotation of the player instead of the controller, so the camera turns along and looks down on the player a bit
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->SetRelativeRotation(FRotator(-25.f, 0.f, 0.f));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	Body->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	//The engine sphere has a radius of 50, the same as the capsule
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Body->SetStaticMesh(SphereMesh.Object);
	}

	//This material has a "Color" parameter we use to make the sphere white
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (ShapeMaterial.Succeeded())
	{
		Body->SetMaterial(0, ShapeMaterial.Object);
	}

	AbilitySystemComponent = CreateDefaultSubobject<UExclusiveAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

void APlayerCharacter::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);

	if (UMaterialInstanceDynamic* Material = Body->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), BodyColor);
	}
}

void APlayerCharacter::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	TickTurn(deltaTime);
	TickStep(deltaTime);
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APlayerCharacter::Respawn(const FTransform& respawnTransform)
{
	//Cancelling reverts the effect of the active ability, like the levitation or the time slowdown
	AbilitySystemComponent->CancelAllAbilities();
	StopStepAndTurn();

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

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnMoveInput);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayerCharacter::OnMoveInputCompleted);

	//Binds the input actions of the ability descriptions, also the ones granted later on
	AbilitySystemComponent->BindAbilityInput(EnhancedInputComponent);
}

void APlayerCharacter::OnMoveInput(const FInputActionValue& value)
{
	const FVector2D MoveInput = value.Get<FVector2D>();

	//Triggered fires every frame while a key is held, only react to the axes that just got pressed
	const bool bStepPressed = !FMath::IsNearlyZero(MoveInput.Y) && FMath::IsNearlyZero(PreviousMoveInput.Y);
	const bool bTurnPressed = !FMath::IsNearlyZero(MoveInput.X) && FMath::IsNearlyZero(PreviousMoveInput.X);
	PreviousMoveInput = MoveInput;

	//Finish the current step or turn first, so the player always ends up on the same grid
	if (IsStepping() || IsTurning())
	{
		return;
	}

	if (bStepPressed)
	{
		StartStep(FMath::Sign(MoveInput.Y));
	}
	else if (bTurnPressed)
	{
		StartTurn(90.f * FMath::Sign(MoveInput.X));
	}
}

void APlayerCharacter::OnMoveInputCompleted(const FInputActionValue& value)
{
	PreviousMoveInput = FVector2D::ZeroVector;
}

void APlayerCharacter::StartStep(float direction)
{
	StepDirection = GetActorForwardVector().GetSafeNormal2D() * direction;
	RemainingStepDistance = StepDistance;
}

void APlayerCharacter::StartTurn(float angle)
{
	TargetYaw = GetActorRotation().Yaw + angle;
	RemainingTurnAngle = angle;
}

void APlayerCharacter::StopStepAndTurn()
{
	RemainingStepDistance = 0.f;
	RemainingTurnAngle = 0.f;
}

void APlayerCharacter::TickStep(float deltaTime)
{
	if (!IsStepping())
	{
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	const float Distance = FMath::Min(StepSpeed * deltaTime, RemainingStepDistance);
	RemainingStepDistance -= Distance;

	//Only move horizontally, the movement component keeps handling gravity and the levitation keeps handling the height.
	//Sweep so walls and the Step block the player, the step ends early when something is in the way.
	FHitResult Hit;
	Movement->SafeMoveUpdatedComponent(StepDirection * Distance, GetActorQuat(), true, Hit);
	if (Hit.bBlockingHit)
	{
		RemainingStepDistance = 0.f;
	}
}

void APlayerCharacter::TickTurn(float deltaTime)
{
	if (!IsTurning())
	{
		return;
	}

	const float MaxAngle = TurnSpeed * deltaTime;
	const float Angle = FMath::Clamp(RemainingTurnAngle, -MaxAngle, MaxAngle);
	RemainingTurnAngle -= Angle;

	//Snap to the target once done, so turning many times doesn't build up rounding errors
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw = IsTurning() ? NewRotation.Yaw + Angle : TargetYaw;
	SetActorRotation(NewRotation);
}
