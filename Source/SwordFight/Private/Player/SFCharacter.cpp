// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SFCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "DrawDebugHelpers.h"

// Sets default values
ASFCharacter::ASFCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(GetMesh());
	SpringArmComp->TargetArmLength = 300.f;
	SpringArmComp->SocketOffset = {0.f, 70.f, 0.f};
	SpringArmComp->SetRelativeLocation({0.f, 0.f, 140.f});

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// defaults
	MaxRunSpeed = 480.f;
	MaxSprintSpeed = 620.f;
	CurrentMaxSpeed = MaxRunSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MaxRunSpeed;

	RelativeSpeedIdleTreshold = 0.02f;
	RelativeSpeedSprintStartTreshold = 1.1f;
	RelativeSpeedSprintMaxTreshold = 1.95f;
}

// Called when the game starts or when spawned
void ASFCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASFCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Controller)
	{
		CalcRelativeYaw();
		CalcRelativeSpeed();

		FVector DrawDebugLocation = GetCapsuleComponent()->GetComponentLocation();
		DrawDebugLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		constexpr float MaxArrowLength = 75.f;

		// debug circle
		DrawDebugCircle(GetWorld(), DrawDebugLocation, 50.f, 36, FColor::Cyan, false, -1.f, 0, 2.f, FVector(0.f, 1.f, 0.f), FVector(1.f, 0.f, 0.f));
		DrawDebugPoint(GetWorld(), DrawDebugLocation + FVector(MaxArrowLength, 0.f, 0.f), 5.f, FColor::Red, false, -1.f);
		DrawDebugPoint(GetWorld(), DrawDebugLocation + FVector(0.f, MaxArrowLength, 0.f), 5.f, FColor::Green, false, -1.f);

		// debug actor fwd vec
		FVector ActorDirection = FQuatRotationMatrix(GetActorQuat()).GetScaledAxis(EAxis::X) * MaxArrowLength;
		DrawDebugDirectionalArrow(GetWorld(), DrawDebugLocation, DrawDebugLocation + ActorDirection, 10.f, FColor::Blue, false, -1.f, 0, 2.f);

		// debug camera vec
		FVector CameraDirection = CameraComp->GetComponentTransform().GetUnitAxis(EAxis::X);
		CameraDirection.Z = 0.f;
		CameraDirection.Normalize();
		CameraDirection *= MaxArrowLength;
		DrawDebugDirectionalArrow(GetWorld(), DrawDebugLocation, DrawDebugLocation + CameraDirection, 10.f, FColor::Yellow, false, -1.f, 0, 2.f);
	
		// debug movement direction vec
		FVector MovementDirection = GetCharacterMovement()->Velocity;
		MovementDirection.Normalize();
		MovementDirection *= RelativeSpeed * MaxArrowLength;
		DrawDebugDirectionalArrow(GetWorld(), DrawDebugLocation, DrawDebugLocation + MovementDirection, 10.f, FColor::Cyan, false, -1.f, 0, 2.f);
		
		// debug reltaive yaw
		FString DbgMsg = FString("Yaw: ") + FString::SanitizeFloat(RelativeYaw) + FString("\nSpeed: ") + FString::SanitizeFloat(RelativeSpeed);
		DrawDebugString(GetWorld(), DrawDebugLocation, DbgMsg, nullptr, FColor::Magenta, 0.f, true);
	}

}

// Called to bind functionality to input
void ASFCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ASFCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASFCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASFCharacter::OnStartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASFCharacter::OnStopSprint);

}

void ASFCharacter::MoveForward(float Val)
{
	InputMoveForward = Val;

	if (Controller && Val != 0.f)
	{
		const FRotator Rotation = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Val);
	}
}

void ASFCharacter::MoveRight(float Val)
{
	InputMoveRight = Val;

	if (Controller && Val != 0.f)
	{
		const FRotator Rotation = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Val);
	}
}

void ASFCharacter::OnStartSprint()
{
	SetSprinting(true);
}

void ASFCharacter::OnStopSprint()
{
	SetSprinting(false);
}

FORCEINLINE void ASFCharacter::CalcRelativeYaw()
{
	float InputYaw = FMath::Atan2(-InputMoveRight, InputMoveForward) * (180.f / PI);
	float CapsuleCameraYaw_Delta = CameraComp->GetComponentRotation().Yaw - GetCapsuleComponent()->GetComponentRotation().Yaw;
	CapsuleCameraYaw_Delta = FRotator::NormalizeAxis(CapsuleCameraYaw_Delta); // clamp angle
	RelativeYaw = CapsuleCameraYaw_Delta - InputYaw;
}

FORCEINLINE void ASFCharacter::CalcRelativeSpeed()
{
	const float Velocity2D = GetVelocity().Size2D();

	if (Velocity2D > MaxRunSpeed)
	{
		if (Velocity2D > MaxSprintSpeed)
		{
			RelativeSpeed = 2.f;
			return;
		}
		RelativeSpeed = 1.0f + (Velocity2D - MaxRunSpeed) / (MaxSprintSpeed - MaxRunSpeed);
	}
	else
	{
		RelativeSpeed = Velocity2D / MaxRunSpeed;
	}
}

FORCEINLINE void ASFCharacter::SetSprinting(bool bInShouldSprint)
{
	if (bShouldSprint != bInShouldSprint && GetCharacterMovement())
	{
		if (bInShouldSprint)
		{
			CurrentMaxSpeed = MaxSprintSpeed;
			bShouldSprint = true;
		}
		else
		{
			CurrentMaxSpeed = MaxRunSpeed;
			bShouldSprint = false;
		}

		GetCharacterMovement()->MaxWalkSpeed = CurrentMaxSpeed;
	}

}