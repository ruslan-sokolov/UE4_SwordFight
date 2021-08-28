// Fill out your copyright notice in the Description page of Project Settings. FName("DERPECATED_AnimNotify


#include "Player/SFCharacterAnim.h"

#include "Player/SFCharacter.h"
#include "DrawDebugHelpers.h"

USFCharacterAnim::USFCharacterAnim()
{
	// defaults
	IKFootRBoneName = FName("ball_r"); // this is experimental solution, it's more obvious to use foot_r
	IKFootLBoneName = FName("ball_l");
	HipBoneName = FName("Pelvis");
	RootBoneName= FName("Root");
	
	//DEPRECATED
	NotifierName_AllowIKLegR = FName("AnimNotify_DEPRECATED_Allow_IK_FootR");
	NotifierName_BlockIKLegR = FName("AnimNotify_DEPRECATED_Block_IK_FootR");
	NotifierName_AllowIKLegL = FName("AnimNotify_DEPRECATED_Allow_IK_FootL");
	NotifierName_BlockIKLegL = FName("AnimNotify_DEPRECATED_Block_IK_FootL");

	FootBoneZOffset = 10.f;
	MaxFootIKTraceDist = 50.f;
	MaxHipDisplacement = -36.f;
	HipDisplacementSpeed = 7.f;
	MaxIKLegZ = 40.f;

	FootIKTraceChannel = ECollisionChannel::ECC_Camera;
}

void USFCharacterAnim::NativeInitializeAnimation()
{
	SFCharacter = Cast<ASFCharacter>(TryGetPawnOwner());

	IKFootTraceQueryParams.AddIgnoredActor(SFCharacter);
}

//DEPRECATED
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
	CalcLegIKAlphaValues(DeltaSeconds);

	#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		DrawDebugFootIK();
	#endif
}

FORCEINLINE void USFCharacterAnim::CalcMeshBottomFootPositions()
{
	const FVector BoneFootLocation_R = SFCharacter->GetMesh()->GetSocketLocation(IKFootRBoneName);
	const FVector BoneFootLocation_L = SFCharacter->GetMesh()->GetSocketLocation(IKFootLBoneName);
	
	const FVector BaseOffsetZVec = SFCharacter->GetMesh()->GetComponentScale() * FootBoneZOffset;  // char scale compensate
	const FVector FootBottomOffset_R = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootRBoneName).GetForwardVector() * BaseOffsetZVec;
	const FVector FootBottomOffset_L = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootLBoneName).GetForwardVector() * BaseOffsetZVec;

	MeshBottomFootR = BoneFootLocation_R + FootBottomOffset_R;  // wtf???
	MeshBottomFootL = BoneFootLocation_L - FootBottomOffset_L;

	const FVector RootBoneLocation = SFCharacter->GetMesh()->GetSocketLocation(RootBoneName);

	MeshBottomFootR_RootZ = FVector(MeshBottomFootR.X, MeshBottomFootR.Y, RootBoneLocation.Z);
	MeshBottomFootL_RootZ = FVector(MeshBottomFootL.X, MeshBottomFootL.Y, RootBoneLocation.Z);
}

FORCEINLINE bool USFCharacterAnim::HandleLineTraceFoot(const FVector& InMeshBottomFoot_RootZ, FHitResult& OutFootTraceResult, float& OutFootOffset)
{
	const FVector IKFoot_Start = FVector(InMeshBottomFoot_RootZ.X, InMeshBottomFoot_RootZ.Y, InMeshBottomFoot_RootZ.Z + MaxFootIKTraceDist);
	const FVector IKFoot_End = FVector(InMeshBottomFoot_RootZ.X, InMeshBottomFoot_RootZ.Y, InMeshBottomFoot_RootZ.Z - MaxFootIKTraceDist);

	if (GetWorld()->LineTraceSingleByChannel(OutFootTraceResult, IKFoot_Start, IKFoot_End, FootIKTraceChannel, IKFootTraceQueryParams))
	{
		OutFootOffset = OutFootTraceResult.ImpactPoint.Z - InMeshBottomFoot_RootZ.Z;

		// debug
		//DrawDebugSphere(GetWorld(), OutFootTraceResult.ImpactPoint, 4, 36, FColor::Green, false, -1.f, 0);
		//DrawDebugLine(GetWorld(), IKFoot_Start, IKFoot_End, FColor::Blue, false, -1.f, 0);

		return true;
	}
	else
	{
		// debug
		//DrawDebugLine(GetWorld(), IKFoot_Start, IKFoot_End, FColor::Red, false, -1.f, 0);
		OutFootOffset = 0;
		return false;
	}
}

FORCEINLINE void USFCharacterAnim::CalcLegIKAlphaValues(const float DeltaSeconds)
{
	// line trace Right foot ik position
	FHitResult IKFootTraceResult_R;
	float IKFootOffset_R = 0.f;
	bool IKFootTraceHit_R = HandleLineTraceFoot(MeshBottomFootR_RootZ, IKFootTraceResult_R, IKFootOffset_R);

	// line trace Left foot ik position
	FHitResult IKFootTraceResult_L;
	float IKFootOffset_L = 0.f;
	bool IKFootTraceHit_L = HandleLineTraceFoot(MeshBottomFootL_RootZ, IKFootTraceResult_L, IKFootOffset_L);

	// hip displacement to compensate leg ik works nice only when both feets on the ground OR when idle animation
	if (SFCharacter->IsMoving())
	{
		IKLegHipDisplacementZ_To = 0.f;  // smoothly restore hip displacement back to original position
		IKLegHipDisplacementZ = FMath::FInterpTo(IKLegHipDisplacementZ, IKLegHipDisplacementZ_To, DeltaSeconds, HipDisplacementSpeed);
	
		// calc ik foot alphas, compensate hip displacement until it's negative
		IKAlphaLegRight = (IKFootOffset_R - IKLegHipDisplacementZ) / MaxIKLegZ;
		IKAlphaLegLeft = (IKFootOffset_L  - IKLegHipDisplacementZ) / MaxIKLegZ;
	}
	else
	{
		if (IKFootOffset_R < 0.f && IKFootOffset_R < IKFootOffset_L)
		{
			IKLegHipDisplacementZ_To = FMath::Max(IKFootOffset_R, MaxHipDisplacement);  // negative hip z offset, to make both feet stand on ground
			IKLegHipDisplacementZ = FMath::FInterpTo(IKLegHipDisplacementZ, IKLegHipDisplacementZ_To, DeltaSeconds, HipDisplacementSpeed);  // smooth

			IKFootOffset_L -= IKLegHipDisplacementZ;  // positive z offset for opposite foot
			IKFootOffset_R = 0.f;  // hip displacement set, nullify this value to compensate height
		}
		else if (IKFootOffset_L < 0.f)
		{
			IKLegHipDisplacementZ_To = FMath::Max(IKFootOffset_L, MaxHipDisplacement);  // negative hip z offset, to make both feet stand on ground
			IKLegHipDisplacementZ = FMath::FInterpTo(IKLegHipDisplacementZ, IKLegHipDisplacementZ_To, DeltaSeconds, HipDisplacementSpeed);  // smooth

			IKFootOffset_R -= IKLegHipDisplacementZ;  // positive z offset for opposite foot
			IKFootOffset_L = 0.f;  // hip displacement set, nullify this value to compensate height
		}

		// calc ik foot alphas
		IKAlphaLegRight = IKFootOffset_R / MaxIKLegZ;
		IKAlphaLegLeft = IKFootOffset_L / MaxIKLegZ;
	}
}

FORCEINLINE void USFCharacterAnim::DrawDebugFootIK() const
{
	const FVector IkFootL_Normal = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootLBoneName).GetRightVector();
	const FVector IkFootR_Normal = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootRBoneName).GetRightVector();

	FPlane PlaneL = FPlane(MeshBottomFootL, IkFootL_Normal);
	FPlane PlaneR = FPlane(MeshBottomFootR, IkFootR_Normal);

	// foot plane
	DrawDebugSolidPlane(GetWorld(), PlaneL, MeshBottomFootL, 15, bAllowIKLeg_L ? FColor::Cyan : FColor::Blue);
	DrawDebugSolidPlane(GetWorld(), PlaneR, MeshBottomFootR, 15, bAllowIKLeg_R ? FColor::Cyan : FColor::Blue);

	// foot ik main locations
	// DrawDebugSphere(GetWorld(), SFCharacter->GetMesh()->GetSocketLocation(RootBoneName), 6, 4, FColor::Red);
	DrawDebugSphere(GetWorld(), MeshBottomFootR, 7, 4, FColor::Orange);
	DrawDebugSphere(GetWorld(), MeshBottomFootL, 7, 4, FColor::Orange);
	DrawDebugSphere(GetWorld(), MeshBottomFootR_RootZ, 3, 4, FColor::Yellow, false, -1.f, 0);
	DrawDebugSphere(GetWorld(), MeshBottomFootL_RootZ, 3, 4, FColor::Yellow, false, -1.f, 0);

	// IK Alpha values
	//DrawDebugString(GetWorld(), MeshBottomFootL, FString("IKLegL: ") + FString::SanitizeFloat(IKAlphaLegLeft), 0, FColor::White, 0.f, true);
	//DrawDebugString(GetWorld(), MeshBottomFootR, FString("IKLegR: ") + FString::SanitizeFloat(IKAlphaLegRight), 0, FColor::White, 0.f, true);
	DrawDebugString(GetWorld(), SFCharacter->GetMesh()->GetSocketLocation(HipBoneName), FString("-HipZ: ") + FString::SanitizeFloat(IKLegHipDisplacementZ) + FString(" To: ") + FString::SanitizeFloat(IKLegHipDisplacementZ_To), 0, FColor::Red, 0.f, true);
}
