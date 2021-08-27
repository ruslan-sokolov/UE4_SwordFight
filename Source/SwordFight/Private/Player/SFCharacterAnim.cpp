// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SFCharacterAnim.h"

#include "Player/SFCharacter.h"
#include "DrawDebugHelpers.h"

USFCharacterAnim::USFCharacterAnim()
{
	// defaults
	IKFootRBoneName = FName("Foot_R");
	IKFootLBoneName = FName("Foot_L");
	HipBoneName = FName("Pelvis");
	RootBoneName= FName("Root");

	FootBoneZOffset = 13.5f;
	MaxFootIKTraceDist = 100.f;

	FootIKTraceChannel = ECollisionChannel::ECC_Camera;

	IKFootTraceQueryParams.bTraceComplex = true;
}

void USFCharacterAnim::NativeInitializeAnimation()
{
	SFCharacter = Cast<ASFCharacter>(TryGetPawnOwner());

	IKFootTraceQueryParams.AddIgnoredActor(SFCharacter);
}

void USFCharacterAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	if (SFCharacter == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SFCharacterAnim SFCharacter is nullptr! Please use this anim with SFCharacter as Character!"));
		return;
	}

	CalcMeshBottomFootPositions();
	CalcLegIKAlphaValues();

	DrawDebugFootIK();
}

FORCEINLINE void USFCharacterAnim::CalcMeshBottomFootPositions()
{
	const FVector BoneFootLocation_R = SFCharacter->GetMesh()->GetSocketLocation(IKFootRBoneName);
	const FVector BoneFootLocation_L = SFCharacter->GetMesh()->GetSocketLocation(IKFootLBoneName);
	
	const FVector BaseOffsetZVec = SFCharacter->GetMesh()->GetComponentScale() * FootBoneZOffset;
	const FVector FootBottomOffset_R = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootRBoneName).GetForwardVector() * BaseOffsetZVec;
	const FVector FootBottomOffset_L = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootLBoneName).GetForwardVector() * BaseOffsetZVec;

	MeshBottomFootR = BoneFootLocation_R + FootBottomOffset_R;  // wtf???
	MeshBottomFootL = BoneFootLocation_L - FootBottomOffset_L;
}

FORCEINLINE void USFCharacterAnim::LineTraceFoot(const FVector& InInitialFootPosition, FHitResult& InFootTraceResult)
{
	const FVector IKFoot_Start = FVector(InInitialFootPosition.X, InInitialFootPosition.Y, InInitialFootPosition.Z + MaxFootIKTraceDist);
	const FVector IKFoot_End = FVector(InInitialFootPosition.X, InInitialFootPosition.Y, InInitialFootPosition.Z - MaxFootIKTraceDist);

	GetWorld()->LineTraceSingleByChannel(InFootTraceResult, IKFoot_Start, IKFoot_End, FootIKTraceChannel, IKFootTraceQueryParams);
}

FORCEINLINE void USFCharacterAnim::CalcLegIKAlphaValues()
{
	// todo fix

	const float MagicAlphaValue = 20.f; // todo: remove magic

	const FVector RootBoneLocation = SFCharacter->GetMesh()->GetSocketLocation(RootBoneName);

	// line trace Right foot ik position
	FHitResult IKFootTraceResult_R;
	const FVector IKFootInitialPosition_R = FVector(MeshBottomFootR.X, MeshBottomFootR.Y, MeshBottomFootR.Z + RootBoneLocation.Z);
	 
	if (IKFootTraceResult_R.bBlockingHit)
	{
		IKAlphaLegRight = (IKFootTraceResult_R.Location - MeshBottomFootR).Z / MagicAlphaValue;
	}
	else
	{
		IKAlphaLegRight = 0.f;
	}

	// line trace Left foot ik position
	FHitResult IKFootTraceResult_L;
	const FVector IKFootInitialPosition_L = FVector(MeshBottomFootL.X, MeshBottomFootL.Y, MeshBottomFootL.Z + RootBoneLocation.Z);

	if (IKFootTraceResult_L.bBlockingHit)
	{
		IKAlphaLegLeft = (IKFootTraceResult_L.Location - MeshBottomFootL).Z / MagicAlphaValue;
	}
	else
	{
		IKAlphaLegLeft = 0.f;
	}

	// hip displacement
	IKLegHipDisplacementZ = (IKFootTraceResult_R.Location - IKFootTraceResult_L.Location).Z * -0.5f;
}

FORCEINLINE void USFCharacterAnim::DrawDebugFootIK() const
{
	const FVector IkFootL_Normal = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootLBoneName).GetForwardVector();
	const FVector IkFootR_Normal = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootRBoneName).GetForwardVector();

	FPlane PlaneL = FPlane(MeshBottomFootL, IkFootL_Normal);
	FPlane PlaneR = FPlane(MeshBottomFootR, IkFootR_Normal);

	DrawDebugSolidPlane(GetWorld(), PlaneL, MeshBottomFootL, 15, FColor::Cyan);
	DrawDebugSolidPlane(GetWorld(), PlaneR, MeshBottomFootR, 15, FColor::Cyan);

	DrawDebugString(GetWorld(), MeshBottomFootL, FString("IKLegAlpha: ") + FString::SanitizeFloat(IKAlphaLegLeft), 0, FColor::White, 0.f, true);
	DrawDebugString(GetWorld(), MeshBottomFootR, FString("IKLegAlpha: ") + FString::SanitizeFloat(IKAlphaLegRight), 0, FColor::White, 0.f, true);
	DrawDebugString(GetWorld(), SFCharacter->GetMesh()->GetSocketLocation(HipBoneName), FString("HipDisplacement: ") + FString::SanitizeFloat(IKLegHipDisplacementZ), 0, FColor::White, 0.f, true);
}
