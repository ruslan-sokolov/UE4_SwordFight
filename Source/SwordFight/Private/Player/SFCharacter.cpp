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
		FVector DrawDebugLocation = GetCapsuleComponent()->GetComponentLocation();
		DrawDebugLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		// debug circle
		DrawDebugCircle(GetWorld(), DrawDebugLocation, 50.f, 36, FColor::Red, false, -1.f, 0, 2.f, FVector(0.f, 1.f, 0.f), FVector(1.f, 0.f, 0.f));

		// debug actor fwd vec
		FVector ActorDirection = FQuatRotationMatrix(GetActorQuat()).GetScaledAxis(EAxis::X) * 75.f;
		DrawDebugDirectionalArrow(GetWorld(), DrawDebugLocation, DrawDebugLocation + ActorDirection, 10.f, FColor::Blue, false, -1.f, 0, 2.f);

		// debug camera vec
		FVector CameraDirection = CameraComp->GetComponentTransform().GetUnitAxis(EAxis::X);
		CameraDirection.Z = 0.f;
		CameraDirection.Normalize();
		CameraDirection *= 75.f;
		DrawDebugDirectionalArrow(GetWorld(), DrawDebugLocation, DrawDebugLocation + CameraDirection, 10.f, FColor::Yellow, false, -1.f, 0, 2.f);
	
		// debug movement direction vec
		FVector MovementDirection = GetCharacterMovement()->Velocity;
		MovementDirection.Normalize();
		MovementDirection *= (GetCharacterMovement()->Velocity.Size() / GetCharacterMovement()->MaxWalkSpeed) * 75.f;;
		DrawDebugDirectionalArrow(GetWorld(), DrawDebugLocation, DrawDebugLocation + MovementDirection, 10.f, FColor::Cyan, false, -1.f, 0, 2.f);

		// calc relative capsule-camera yaw
		CalcRelativeYaw();
		
		// debug reltaive yaw
		DrawDebugString(GetWorld(), DrawDebugLocation, FString("Yaw: ") + FString::SanitizeFloat(RelativeYaw, 1), nullptr , FColor::Magenta, 0.f, true, 1.5f);
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

FORCEINLINE void ASFCharacter::CalcRelativeYaw()
{
	float InputYaw = FMath::Atan2(-InputMoveRight, InputMoveForward) * (180.f / PI);
	float CapsuleCameraYaw_Delta = CameraComp->GetComponentRotation().Yaw - GetCapsuleComponent()->GetComponentRotation().Yaw;
	CapsuleCameraYaw_Delta = FRotator::NormalizeAxis(CapsuleCameraYaw_Delta); // clamp angle
	RelativeYaw = CapsuleCameraYaw_Delta - InputYaw;
}
