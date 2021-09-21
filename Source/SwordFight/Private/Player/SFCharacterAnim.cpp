// Fill out your copyright notice in the Description page of Project Settings. FName("DERPECATED_AnimNotify


#include "Player/SFCharacterAnim.h"

#include "Player/SFCharacter.h"
#include "DrawDebugHelpers.h"

#include <SwordFight/Public/Weapons/SFWeapon.h>

USFCharacterAnim::USFCharacterAnim()
{
	// defaults
	IKFootRBoneName = FName("Foot_R");
	IKFootLBoneName = FName("Foot_L");
	HipBoneName = FName("Pelvis");
	RootBoneName= FName("Root");
	
	NotifierName_AllowIKLegR = FName("AnimNotify_Allow_IK_FootR");
	NotifierName_BlockIKLegR = FName("AnimNotify_Block_IK_FootR");
	NotifierName_AllowIKLegL = FName("AnimNotify_Allow_IK_FootL");
	NotifierName_BlockIKLegL = FName("AnimNotify_Block_IK_FootL");

	FootBoneZOffset = 13.5f;
	MaxFootIKTraceDist = 50.f;
	MaxHipDisplacement = -36.f;
	HipDisplacementSpeed = 7.f;
	MaxIKLegZ = 36.f;

	FootIKTraceChannel = ECollisionChannel::ECC_Camera;
}

void USFCharacterAnim::NativeInitializeAnimation()
{
	SFCharacter = Cast<ASFCharacter>(TryGetPawnOwner());

	IKFootTraceQueryParams.AddIgnoredActor(SFCharacter);
}

bool USFCharacterAnim::HandleNotify(const FAnimNotifyEvent& AnimNotifyEvent)
{
	// handle ik leg allow/block notifiers
	if (AnimNotifyEvent.GetNotifyEventName() == NotifierName_AllowIKLegR)
	{
		bAllowIKLeg_R = true;
	}
	else if (AnimNotifyEvent.GetNotifyEventName() == NotifierName_BlockIKLegR)
	{
		bAllowIKLeg_R = false;
	}
	else if (AnimNotifyEvent.GetNotifyEventName() == NotifierName_AllowIKLegL)
	{
		bAllowIKLeg_L = true;
	}
	else if (AnimNotifyEvent.GetNotifyEventName() == NotifierName_BlockIKLegL)
	{
		bAllowIKLeg_L = false;
	}

	return false;
}

void USFCharacterAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	if (SFCharacter == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SFCharacterAnim SFCharacter is nullptr! Please use this anim with SFCharacter as Character!"));
		return;
	}

	CalcMeshBottomFootPositions();
	CalcIKValues(DeltaSeconds);
	UpdateWantToBlock();

	#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		DrawDebugFootIK();
	#endif
}

FORCEINLINE void USFCharacterAnim::CalcMeshBottomFootPositions()
{
	CharacterScale = SFCharacter->GetMesh()->GetComponentScale(); // cache character scale

	const FVector BoneFootLocation_R = SFCharacter->GetMesh()->GetSocketLocation(IKFootRBoneName);
	const FVector BoneFootLocation_L = SFCharacter->GetMesh()->GetSocketLocation(IKFootLBoneName);
	
	const FVector BaseOffsetZVec = CharacterScale * FootBoneZOffset;  // char scale compensate
	const FVector FootBottomOffset_R = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootRBoneName).GetForwardVector() * BaseOffsetZVec;
	const FVector FootBottomOffset_L = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootLBoneName).GetForwardVector() * BaseOffsetZVec;

	MeshBottomFootR = BoneFootLocation_R + FootBottomOffset_R;  // ball_r right vector directs upward
	MeshBottomFootL = BoneFootLocation_L - FootBottomOffset_L;  // ball_l right vector directs downward

	const FVector RootBoneLocation = SFCharacter->GetMesh()->GetSocketLocation(RootBoneName);

	MeshBottomFootR_RootZ = FVector(MeshBottomFootR.X, MeshBottomFootR.Y, RootBoneLocation.Z);  // cache bottom foot mesh location 
	MeshBottomFootL_RootZ = FVector(MeshBottomFootL.X, MeshBottomFootL.Y, RootBoneLocation.Z);
}

FORCEINLINE bool USFCharacterAnim::HandleLineTraceFoot(const FVector& InMeshBottomFoot_RootZ, FHitResult& OutFootTraceResult, float& OutFootOffset)
{
	const FVector IKFoot_Start = FVector(InMeshBottomFoot_RootZ.X, InMeshBottomFoot_RootZ.Y, InMeshBottomFoot_RootZ.Z + MaxFootIKTraceDist * CharacterScale.Z);
	const FVector IKFoot_End = FVector(InMeshBottomFoot_RootZ.X, InMeshBottomFoot_RootZ.Y, InMeshBottomFoot_RootZ.Z - MaxFootIKTraceDist * CharacterScale.Z);

	if (GetWorld()->LineTraceSingleByChannel(OutFootTraceResult, IKFoot_Start, IKFoot_End, FootIKTraceChannel, IKFootTraceQueryParams))
	{
		OutFootOffset = OutFootTraceResult.ImpactPoint.Z - InMeshBottomFoot_RootZ.Z;
		
		return true;
	}
	else
	{
		OutFootOffset = 0;
		return false;
	}
}

FORCEINLINE void USFCharacterAnim::CalcFootRotation(const FHitResult& FootTrace_R, const FHitResult& FootTrace_L)
{
	if (bAllowIKLeg_R)
	{
		const float Pitch_R = -FMath::RadiansToDegrees(FMath::Atan2(FootTrace_R.ImpactNormal.X, FootTrace_R.ImpactNormal.Z));
		const float Roll_R = FMath::RadiansToDegrees(FMath::Atan2(FootTrace_R.ImpactNormal.Y, FootTrace_R.ImpactNormal.Z));
		FootRightWorldRotation = FRotator(Pitch_R, 0.f, Roll_R);
	}
	else
	{
		FootRightWorldRotation = FRotator(0.f);
	}

	if (bAllowIKLeg_L)
	{
		const float Pitch_L = -FMath::RadiansToDegrees(FMath::Atan2(FootTrace_L.ImpactNormal.X, FootTrace_L.ImpactNormal.Z));
		const float Roll_L = FMath::RadiansToDegrees(FMath::Atan2(FootTrace_L.ImpactNormal.Y, FootTrace_L.ImpactNormal.Z));
		FootLeftWorldRotation = FRotator(Pitch_L, 0.f, Roll_L);
	}
	else
	{
		FootLeftWorldRotation = FRotator(0.f);
	}
}

FORCEINLINE void USFCharacterAnim::CalcIKValues(const float DeltaSeconds)
{
	// line trace Right foot ik position
	FHitResult IKFootTraceResult_R;
	float IKFootOffset_R = 0.f;
	bool IKFootTraceHit_R = HandleLineTraceFoot(MeshBottomFootR_RootZ, IKFootTraceResult_R, IKFootOffset_R);

	// line trace Left foot ik position
	FHitResult IKFootTraceResult_L;
	float IKFootOffset_L = 0.f;
	bool IKFootTraceHit_L = HandleLineTraceFoot(MeshBottomFootL_RootZ, IKFootTraceResult_L, IKFootOffset_L);

	CalcFootRotation(IKFootTraceResult_R, IKFootTraceResult_L);  // calc foot rotation

	// hip displacement to compensate leg ik works nice only when both feets on the ground OR when idle animation
	if (SFCharacter->IsMoving())
	{
		IKLegHipDisplacementZ_To = 0.f;  // smoothly restore hip displacement back to original position
		IKLegHipDisplacementZ = FMath::FInterpTo(IKLegHipDisplacementZ, IKLegHipDisplacementZ_To, DeltaSeconds, HipDisplacementSpeed * CharacterScale.Z);
	
		// calc ik foot alphas, compensate hip displacement until it's negative
		IKAlphaLegRight = (IKFootOffset_R - IKLegHipDisplacementZ) / (MaxIKLegZ * CharacterScale.Z);
		IKAlphaLegLeft = (IKFootOffset_L  - IKLegHipDisplacementZ) / (MaxIKLegZ * CharacterScale.Z);
	}
	else
	{
		if (IKFootOffset_R < 0.f && IKFootOffset_R < IKFootOffset_L)
		{
			IKLegHipDisplacementZ_To = FMath::Max(IKFootOffset_R, MaxHipDisplacement * CharacterScale.Z);  // negative hip z offset, to make both feet stand on ground
			IKLegHipDisplacementZ = FMath::FInterpTo(IKLegHipDisplacementZ, IKLegHipDisplacementZ_To, DeltaSeconds, HipDisplacementSpeed * CharacterScale.Z);  // smooth

			IKFootOffset_L -= IKLegHipDisplacementZ;  // positive z offset for opposite foot
			IKFootOffset_R = 0.f;  // hip displacement set, nullify this value to compensate height
		}
		else if (IKFootOffset_L < 0.f)
		{
			IKLegHipDisplacementZ_To = FMath::Max(IKFootOffset_L, MaxHipDisplacement * CharacterScale.Z);  // negative hip z offset, to make both feet stand on ground
			IKLegHipDisplacementZ = FMath::FInterpTo(IKLegHipDisplacementZ, IKLegHipDisplacementZ_To, DeltaSeconds, HipDisplacementSpeed * CharacterScale.Z);  // smooth

			IKFootOffset_R -= IKLegHipDisplacementZ;  // positive z offset for opposite foot
			IKFootOffset_L = 0.f;  // hip displacement set, nullify this value to compensate height
		}

		// calc ik leg alphas
		IKAlphaLegRight = IKFootOffset_R / (MaxIKLegZ * CharacterScale.Z);
		IKAlphaLegLeft = IKFootOffset_L / (MaxIKLegZ * CharacterScale.Z);
	}
}

FORCEINLINE void USFCharacterAnim::DrawDebugFootIK() const
{
	{
		// foot plane
		auto IKFootL_Rot = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootLBoneName);
		auto IKFootR_Rot = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootRBoneName);
		FQuat Rotation = FQuat(FRotator(0.f, 90.f, 90.f));
	
		const FVector FootPlaneBox(17.f * CharacterScale.X, 13.f * CharacterScale.Y, 0.05f);

		DrawDebugSolidBox(GetWorld(), MeshBottomFootL, FootPlaneBox, IKFootL_Rot * Rotation, bAllowIKLeg_L ? FColor::Cyan : FColor::Blue);
		DrawDebugSolidBox(GetWorld(), MeshBottomFootR, FootPlaneBox, IKFootR_Rot * Rotation, bAllowIKLeg_R ? FColor::Cyan : FColor::Blue);
	}

	{
		// foot ik main locations
		float XYScale = CharacterScale.Size2D();
		// DrawDebugSphere(GetWorld(), SFCharacter->GetMesh()->GetSocketLocation(RootBoneName), 6, 4, FColor::Red);
		DrawDebugSphere(GetWorld(), MeshBottomFootR, 7 * XYScale, 4, FColor::Orange);
		DrawDebugSphere(GetWorld(), MeshBottomFootL, 7 * XYScale, 4, FColor::Orange);
		DrawDebugSphere(GetWorld(), MeshBottomFootR_RootZ, 3 * XYScale, 4, FColor::Yellow, false, -1.f, 0);
		DrawDebugSphere(GetWorld(), MeshBottomFootL_RootZ, 3 * XYScale, 4, FColor::Yellow, false, -1.f, 0);
	}

	{
		// IK calculated values
		FString LegRMsg = "IKLegR: " + FString::SanitizeFloat(IKAlphaLegRight) + "\n FootRotR: " + FootRightWorldRotation.ToString();
		FString LegLMsg = "IKLegL: " + FString::SanitizeFloat(IKAlphaLegLeft) + "\n FootRotL" + FootLeftWorldRotation.ToString();
		DrawDebugString(GetWorld(), MeshBottomFootR, LegRMsg, 0, FColor::White, 0.f, true);
		DrawDebugString(GetWorld(), MeshBottomFootL, LegLMsg, 0, FColor::White, 0.f, true);

		FString HipMsg = "-HipZ: " + FString::SanitizeFloat(IKLegHipDisplacementZ) + " To: " + FString::SanitizeFloat(IKLegHipDisplacementZ_To);
		DrawDebugString(GetWorld(), SFCharacter->GetMesh()->GetSocketLocation(HipBoneName), HipMsg, 0, FColor::Red, 0.f, true);
	}
}

FORCEINLINE void USFCharacterAnim::UpdateWantToBlock()
{
	if (SFCharacter)
	{
		if (ASFWeapon* Shield = SFCharacter->GetWeaponInLeftHand())
		{
			bWantToBlock = Shield->IsBlocking();
		}
	}
}