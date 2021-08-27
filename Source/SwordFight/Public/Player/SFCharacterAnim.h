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
	
	// calculate right offset in 3d space for foot mesh bottom using FootBoneZOffset
	FORCEINLINE void CalcMeshBottomFootPositions();
	
	/** Handle line trace for foot
	
	* @param InInitialFootPosition : where foot is should be located if IK disabled. should be vector: bone location xy and root z
	* @param InFootTraceResult : foot ik trace result out
	*/
	FORCEINLINE void LineTraceFoot(const FVector& InInitialFootPosition, FHitResult& InFootTraceResult);

	// calculates IKAlphaLegLeft, IKAlphaLegRight, IKLegHipDisplacement
	FORCEINLINE void CalcLegIKAlphaValues();

	// default IK Foot trace query params
	FCollisionQueryParams IKFootTraceQueryParams;

public:
	USFCharacterAnim();

	UPROPERTY(EditDefaultsOnly)
	FName IKFootRBoneName;

	UPROPERTY(EditDefaultsOnly)
	FName IKFootLBoneName;

	UPROPERTY(EditDefaultsOnly)
	FName HipBoneName;

	UPROPERTY(EditDefaultsOnly)
	FName RootBoneName;
	
	// z distance between foot bone and foot mesh bottom part (applied to default scale size, actual value is change depended on scale)
	UPROPERTY(EditDefaultsOnly)
	float FootBoneZOffset;

	// max z distance up and down from root to trace foot ik
	UPROPERTY(EditDefaultsOnly)
	float MaxFootIKTraceDist;

	//UPROPERTY(EditDefaultsOnly)
	ECollisionChannel FootIKTraceChannel;


	UPROPERTY(BlueprintReadOnly)
	class ASFCharacter* SFCharacter;

	// IK alpha value for right foot LegIK
	UPROPERTY(BlueprintReadOnly)
	float IKAlphaLegRight;

	// IK alpha value for left foot LegIK
	UPROPERTY(BlueprintReadOnly)
	float IKAlphaLegLeft;

	// Hip bone z displacement during LegIK to allow one leg be relatively lower than root bone
	UPROPERTY(BlueprintReadOnly)
	float IKLegHipDisplacementZ;

	// right foot mesh bottom part position (stable to mesh scale);
	UPROPERTY(BlueprintReadOnly)
	FVector MeshBottomFootR;

	// left foot mesh bottom part position (stable to mesh scale);
	UPROPERTY(BlueprintReadOnly)
	FVector MeshBottomFootL;


	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual void NativeInitializeAnimation() override;
	

};
