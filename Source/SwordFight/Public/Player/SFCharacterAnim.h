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
	
	* @param InMeshBottomFoot_RootZ : location of mesh bottom foot on root z level (left or right)
	* @param OutFootTraceResult : foot ik trace result out
	* @param OutFootOffset : distance between foot root location and foot location where it should be
	* @return Success LineTrace
	*/
	FORCEINLINE bool HandleLineTraceFoot(const FVector& InMeshBottomFoot_RootZ, FHitResult& OutFootTraceResult, float& OutFootOffset);

	// calculates IKAlphaLegLeft, IKAlphaLegRight, IKLegHipDisplacement
	FORCEINLINE void CalcLegIKAlphaValues(const float DeltaSeconds);

	// default IK Foot trace query params
	FCollisionQueryParams IKFootTraceQueryParams;

	// cache bottom foot mesh location 
	FVector MeshBottomFootR_RootZ, MeshBottomFootL_RootZ;

	// DEPRECATED animation notifiers ik leg controllers
	bool bAllowIKLeg_R, bAllowIKLeg_L;

	// value to smooth hip z dislocation on leg ik
	float IKLegHipDisplacementZ_To;

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

	//DEPRECATED
	UPROPERTY(EditDefaultsOnly)
	FName NotifierName_AllowIKLegR;
	
	UPROPERTY(EditDefaultsOnly)
	FName NotifierName_BlockIKLegR;
	
	UPROPERTY(EditDefaultsOnly)
	FName NotifierName_AllowIKLegL;
	
	UPROPERTY(EditDefaultsOnly)
	FName NotifierName_BlockIKLegL;
	//
	
	// z distance between foot bone and foot mesh bottom part (applied to default scale size, actual value is change depended on scale)
	UPROPERTY(EditDefaultsOnly)
	float FootBoneZOffset;

	// max z distance up and down from root to trace foot
	UPROPERTY(EditDefaultsOnly)
	float MaxFootIKTraceDist;

	// clamp max "-z hip adjustmenet compensating negative leg IK" value
	UPROPERTY(EditDefaultsOnly)
	float MaxHipDisplacement;

	// speed of changing IKLegHipDisplacementZ smothly
	UPROPERTY(EditDefaultsOnly)
	float HipDisplacementSpeed;

	// maximum IK Leg Z distance, used to calculate IK Leg alpha value
	UPROPERTY(EditDefaultsOnly)
	float MaxIKLegZ;

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
	
protected:
	//DEPRECATED
	virtual bool HandleNotify(const FAnimNotifyEvent& AnimNotifyEvent) override;
};
