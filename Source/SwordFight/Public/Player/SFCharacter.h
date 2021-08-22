// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SFCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class SWORDFIGHT_API ASFCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASFCharacter();

protected:
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmComp;
	
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComp;

	/** Relative Yaw angle between camera capsule component and input direction */
	float RelativeYaw;
	/** -1 if backward, 1 if forward, 2 if fwd sprint, -2 if fwd sprint*/
	float RelativeSpeedForward;
	/** -1 if left, 1 if right, 2 if right sprint, 2 if left sprint */
	float RelativeSpeedRight;

	/** if player sprint action pressed */
	bool bShouldSprint;
	
	/** Character skeletal mesh root bone name */
	UPROPERTY(Category = "SFCharacterAnimation", EditDefaultsOnly)
	FName RootBoneName;

private:
	/** Calculate RelativeYaw angle between */
	FORCEINLINE void CalcRelativeYaw();
	/** Handle run/sprint switch */
	FORCEINLINE void SetSprinting(bool bShouldSprint);

	/** Draw debug movement */
	FORCEINLINE void DrawDebugMovement();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	* Move forward/back
	*
	* @param Val Movment input to apply
	*/
	void MoveForward(float Val);

	/**
	* Strafe right/left
	*
	* @param Val Movment input to apply
	*/
	void MoveRight(float Val);

	/** player pressed sprint action */
	void OnStartSprint();
	
	/** player released sprint action */
	void OnStopSprint();
	
	/** Return relative Yaw angle between camera capsule component and input direction */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE float GetRelativeYaw() const { return RelativeYaw; }

	/** -1 if left, 1 if right, 2 if right sprint, 2 if left sprint */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE float GetRelativeRightSpeed() const { return RelativeSpeedRight; }

	/** -1 if backward, 1 if forward, 2 if fwd sprint, -2 if fwd sprint*/
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE float GetRelativeForwardSpeed() const { return RelativeSpeedForward; }

	/** return 0 if idle 1 if run 2 if sprint */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE float GetAbsMaxSpeed() const { return FMath::Max(FMath::Abs(RelativeSpeedForward), FMath::Abs(RelativeSpeedRight)); }

	/** if player sprint action pressed */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE bool IsShouldSprint() const { return bShouldSprint; }

	/** If relative speed > 0.01f, character is moving */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE bool IsMoving() const { return RelativeSpeedRight != 0.f || RelativeSpeedForward != 0.f; }

	/** if relative speed > 1.98 */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE bool IsSprinting() const { return RelativeSpeedRight >= 2.f || RelativeSpeedForward >= 2.f; }
};
