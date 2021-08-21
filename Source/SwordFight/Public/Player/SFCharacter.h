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
	/** Last MoveForward input value */
	float InputMoveForward;
	/** Last MoveRight input value */
	float InputMoveRight;

private:
	/** Calculate RelativeYaw angle between */
	FORCEINLINE void CalcRelativeYaw();

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
	
	/** Return relative Yaw angle between camera capsule component and input direction */
	UFUNCTION(BlueprintCallable, Category = Character)
	float GetRelativeYaw() const { return RelativeYaw; }

	/** Get last move forward input value */
	UFUNCTION(BlueprintCallable, Category = Character)
	float GetInputMoveForward() const { return InputMoveForward; }

	/** Get last move right input value */
	UFUNCTION(BlueprintCallable, Category = Character)
	float GetInputMoveRight() const { return InputMoveRight; }

};
