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
	/** Relative Speed value, 0 if idle, 1 if speed 2 if sprint, interpolated */
	float RelativeSpeed;
	/** Last MoveForward input value */
	float InputMoveForward;
	/** Last MoveRight input value */
	float InputMoveRight;
	
	/** Current Max Speed (sprint or run etc) */
	float CurrentMaxSpeed;

	/** if player sprint action pressed */
	bool bShouldSprint;

	/** Max character run speed */
	UPROPERTY(Category = "SFCharacter", EditDefaultsOnly)
	float MaxRunSpeed;
	/** Max character sprint speed */
	UPROPERTY(Category = "SFCharacter", EditDefaultsOnly)
	float MaxSprintSpeed;
	
	/** if relative speed is less then this value, character anim considered not movable */
	UPROPERTY(Category = "SFCharacterAnimation", EditDefaultsOnly)
	float RelativeSpeedIdleTreshold;
	/** If relative speed is grater then this value, character anim considered start sprinting*/
	UPROPERTY(Category = "SFCharacterAnimation", EditDefaultsOnly)
	float RelativeSpeedSprintStartTreshold;
	/** If reltaive speed is grater than this value, character anim considered sprint and max speed */
	UPROPERTY(Category = "SFCharacterAnimation", EditDefaultsOnly)
	float RelativeSpeedSprintMaxTreshold;

private:
	/** Calculate RelativeYaw angle between */
	FORCEINLINE void CalcRelativeYaw();
	/** Calculate RelativeSpeed */
	FORCEINLINE void CalcRelativeSpeed();
	/** Handle run/sprint switch */
	FORCEINLINE void SetSprinting(bool bShouldSprint);

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

	/** Get last move forward input value */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE float GetInputMoveForward() const { return InputMoveForward; }

	/** Get last move right input value */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE float GetInputMoveRight() const { return InputMoveRight; }

	/** Get Relative Speed value, 0 if idle, 1 if speed 2 if sprint, interpolated with current velocity */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE float GetRelativeSpeed() const { return RelativeSpeed; }

	/** Get Current max speed, return max for run or sprint etc */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE float GetCurrentMaxSpeed() const { return CurrentMaxSpeed; }

	/** if player sprint action pressed */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE bool IsShouldSprint() const { return bShouldSprint; }

	/** If relative speed > RelativeSpeedIdleTreshold, character is moving */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE bool IsMoving() const { return RelativeSpeed > RelativeSpeedIdleTreshold; }

	/** if relative speed > RelativeSpeedSprintStartTreshold */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE bool IsSprinting() const { return RelativeSpeed > RelativeSpeedSprintStartTreshold; }

	/** if relative speed > RelativeSpeedSprintMaxTreshold */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE bool IsSprintingSpeedMax() const { return RelativeSpeed > RelativeSpeedSprintMaxTreshold; }

	/** Get Relative Speed value, 0 if idle, 1 if speed 2 if sprint */
	UFUNCTION(BlueprintCallable, Category = Character)
	FORCEINLINE float GetRelativeSpeedMax() const { return IsMoving() ? (IsSprinting() ? 2.0f : 1.0f) : 0.f; }
};
