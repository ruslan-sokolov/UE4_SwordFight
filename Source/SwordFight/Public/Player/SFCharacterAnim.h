// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SFCharacterAnim.generated.h"

/**
 * 
 */
UCLASS()
class SWORDFIGHT_API USFCharacterAnim : public UAnimInstance
{
	GENERATED_BODY()

private:
	FORCEINLINE void DrawDebugFootIK() const;

	FVector MeshBottomFootR;
	FVector MeshBottomFootL;
	
	// calculate right offset in 3d space for foot mesh bottom using FootBoneZOffset
	FORCEINLINE void CalcMeshBottomFootPositions();

public:
	USFCharacterAnim();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class ASFCharacter* SFCharacter;

	UPROPERTY(EditDefaultsOnly)
	FName IKFootR;

	UPROPERTY(EditDefaultsOnly)
	FName IKFootL;
	
	// z distance between foot bone and foot mesh bottom part (applied to default scale size, actual value is change depended on scale)
	UPROPERTY(EditDefaultsOnly)
	float FootBoneZOffset;

	// get right foot mesh bottom part position (can handle mesh scale);
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FVector GetMeshBottomFootR() const { return MeshBottomFootR; }

	// get left foot mesh bottom part position (can handle mesh scale);
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FVector GetMeshBottomFootL() const { return MeshBottomFootL; }

	virtual void NativeUpdateAnimation(float DeltaSeconds);

	virtual void NativeInitializeAnimation();
	

};
