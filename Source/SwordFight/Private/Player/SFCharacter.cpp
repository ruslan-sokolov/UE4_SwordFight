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
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bAllowPhysicsRotationDuringAnimRootMotion = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	RootBoneName = FName("root");
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
		
		#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			DrawDebugMovement();
		#endif
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
	PlayerInputComponent->BindAction("DisableMoveToCamera", IE_Pressed, this, &ASFCharacter::OnStartDisableMoveToCamera);
	PlayerInputComponent->BindAction("DisableMoveToCamera", IE_Released, this, &ASFCharacter::OnEndDisableMoveToCamera);

}

void ASFCharacter::MoveForward(float Val)
{
	if (Val == 0.f)
	{
		RelativeSpeedForward = 0.f;
	}
	else if (Val > 0.f)
	{
		RelativeSpeedForward = bShouldSprint ? 2.f : 1.f;
	}
	else
	{
		RelativeSpeedForward = bShouldSprint ? -2.f : -1.f;
	}
}

void ASFCharacter::MoveRight(float Val)
{
	if (Val == 0.f)
	{
		RelativeSpeedRight = 0.f;
	}
	else if (Val > 0.f)
	{
		RelativeSpeedRight = bShouldSprint ? 2.f : 1.f;
	}
	else
	{
		RelativeSpeedRight = bShouldSprint ? -2.f : -1.f;
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

void ASFCharacter::OnStartDisableMoveToCamera()
{
	if (GetCharacterMovement())
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void ASFCharacter::OnEndDisableMoveToCamera()
{
	if (GetCharacterMovement())
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
}

FORCEINLINE void ASFCharacter::SetSprinting(bool bInShouldSprint)
{
	bShouldSprint = bInShouldSprint;
}

FORCEINLINE void ASFCharacter::CalcRelativeYaw()
{
	float InputYaw = FMath::Atan2(-RelativeSpeedRight, RelativeSpeedForward) * (180.f / PI);
	float CapsuleCameraYaw_Delta = CameraComp->GetComponentRotation().Yaw - GetCapsuleComponent()->GetComponentRotation().Yaw;
	CapsuleCameraYaw_Delta = FRotator::NormalizeAxis(CapsuleCameraYaw_Delta); // clamp angle
	RelativeYaw = CapsuleCameraYaw_Delta - InputYaw;
}

FORCEINLINE void ASFCharacter::DrawDebugMovement() const
{
	FVector DrawDebugLocation = GetCapsuleComponent()->GetComponentLocation();
	DrawDebugLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	constexpr float MaxArrowLength = 75.f;

	// debug circle
	DrawDebugCircle(GetWorld(), DrawDebugLocation, 50.f, 36, FColor::Cyan, false, -1.f, 0, 0.f, FVector(0.f, 1.f, 0.f), FVector(1.f, 0.f, 0.f));
	DrawDebugPoint(GetWorld(), DrawDebugLocation + FVector(MaxArrowLength, 0.f, 0.f), 5.f, FColor::Red, false, -1.f);
	DrawDebugPoint(GetWorld(), DrawDebugLocation + FVector(0.f, MaxArrowLength, 0.f), 5.f, FColor::Green, false, -1.f);

	// debug actor fwd vec
	auto ActorQuatRotation = GetActorQuat();
	FVector ActorDirection = FQuatRotationMatrix(ActorQuatRotation).GetScaledAxis(EAxis::X) * MaxArrowLength;
	DrawDebugDirectionalArrow(GetWorld(), DrawDebugLocation, DrawDebugLocation + ActorDirection, 10.f, FColor::Blue, false, -1.f, 0, 0.f);

	// debug camera vec
	FVector CameraDirection = CameraComp->GetComponentTransform().GetUnitAxis(EAxis::X);
	CameraDirection.Z = 0.f;
	CameraDirection.Normalize();
	CameraDirection *= MaxArrowLength;
	DrawDebugDirectionalArrow(GetWorld(), DrawDebugLocation, DrawDebugLocation + CameraDirection, 10.f, FColor::Yellow, false, -1.f, 0, 0.f);

	// debug movement direction vec
	FVector InputVector{ RelativeSpeedForward, RelativeSpeedRight, 0.f };
	InputVector.Normalize();
	
	FVector MovementDirection = ActorQuatRotation.RotateVector(InputVector);
	MovementDirection *= GetAbsMaxSpeed() * MaxArrowLength;
	DrawDebugDirectionalArrow(GetWorld(), DrawDebugLocation, DrawDebugLocation + MovementDirection, 10.f, FColor::Cyan, false, -1.f, 0, 0.f);

	// debug reltaive yaw
	FString DbgMsg = FString("Yaw: ") + FString::SanitizeFloat(RelativeYaw) + FString("\nAvg_V: ") + FString::SanitizeFloat(GetCharacterMovement()->Velocity.Size());
	DrawDebugString(GetWorld(), DrawDebugLocation, DbgMsg, nullptr, FColor::Magenta, 0.f, true);

	// debug root
	constexpr float ConeSize = 6.f;
	constexpr float ConeHeight = ConeSize * UE_HALF_SQRT_2;

	const FVector RootDirection = GetMesh()->GetBoneQuaternion(RootBoneName).GetRightVector();
	FVector RootLocation = GetMesh()->GetBoneLocation(RootBoneName);
	RootLocation.Z += ConeHeight;

	const FVector ConeOffset = RootDirection * ConeHeight;
	const FVector FrontConeRootLoc = RootLocation + ConeOffset;
	const FVector BackConeRootLoc = RootLocation - ConeOffset;

	DrawDebugCone(GetWorld(), FrontConeRootLoc, RootDirection, -ConeSize, PI/4.f, PI/4.f, 4, FColor::Red, false, -1.f, 0, 0.f);
	DrawDebugCone(GetWorld(), BackConeRootLoc, RootDirection, ConeSize, PI/4.f, PI/4.f, 4, FColor::Orange, false, -1.f, 0, 0.f);
}