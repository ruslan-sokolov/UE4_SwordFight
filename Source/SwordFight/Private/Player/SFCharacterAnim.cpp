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
	MaxFootIKTraceDist = 50.f;
	MaxHipDisplacement = 36.f;
	MaxIKLegZ = 40.f;

	FootIKTraceChannel = ECollisionChannel::ECC_Camera;
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

		//DrawDebugSphere(GetWorld(), OutFootTraceResult.ImpactPoint, 4, 36, FColor::Green, false, -1.f, 0);
		//DrawDebugLine(GetWorld(), IKFoot_Start, IKFoot_End, FColor::Blue, false, -1.f, 0);

		return true;
	}
	else
	{
		//DrawDebugLine(GetWorld(), IKFoot_Start, IKFoot_End, FColor::Red, false, -1.f, 0);
		OutFootOffset = 0;
		return false;
	}
}

FORCEINLINE void USFCharacterAnim::CalcLegIKAlphaValues()
{
	// line trace Right foot ik position
	FHitResult IKFootTraceResult_R;
	float IKFootOffset_R = 0.f;
	bool IKFootTraceHit_R = HandleLineTraceFoot(MeshBottomFootR_RootZ, IKFootTraceResult_R, IKFootOffset_R);

	// line trace Left foot ik position
	FHitResult IKFootTraceResult_L;
	float IKFootOffset_L = 0.f;
	bool IKFootTraceHit_L = HandleLineTraceFoot(MeshBottomFootL_RootZ, IKFootTraceResult_L, IKFootOffset_L);

	//DrawDebugString(GetWorld(), MeshBottomFootL_RootZ, FString("FootOffsetL: ") + FString::SanitizeFloat(IKFootOffset_L), 0, FColor::White, 0.f, true);
	//DrawDebugString(GetWorld(), MeshBottomFootR_RootZ, FString("FootOffsetR: ") + FString::SanitizeFloat(IKFootOffset_R), 0, FColor::White, 0.f, true);

	if (IKFootOffset_R < 0.f && IKFootOffset_R < IKFootOffset_L)
	{
		IKLegHipDisplacementZ = IKFootOffset_R;  // negative z offset
		IKFootOffset_L -= IKFootOffset_R;  // positive z offset for opposite foot
		IKFootOffset_R = 0.f;  // hip displacement set, nullify this value to compensate height
	}
	else if (IKFootOffset_L < 0.f)
	{
		IKLegHipDisplacementZ = IKFootOffset_L;  // negative z offset
		IKFootOffset_R -= IKFootOffset_L;  // positive z offset for opposite foot
		IKFootOffset_L = 0.f;  // hip displacement set, nullify this value to compensate height
	}
	
	// calc ik foot alphas
	IKAlphaLegRight = IKFootOffset_R / MaxIKLegZ;
	IKAlphaLegLeft = IKFootOffset_L / MaxIKLegZ;
}

FORCEINLINE void USFCharacterAnim::DrawDebugFootIK() const
{
	const FVector IkFootL_Normal = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootLBoneName).GetForwardVector();
	const FVector IkFootR_Normal = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootRBoneName).GetForwardVector();

	FPlane PlaneL = FPlane(MeshBottomFootL, IkFootL_Normal);
	FPlane PlaneR = FPlane(MeshBottomFootR, IkFootR_Normal);

	// foot plane
	//DrawDebugSolidPlane(GetWorld(), PlaneL, MeshBottomFootL, 15, FColor::Cyan);
	//DrawDebugSolidPlane(GetWorld(), PlaneR, MeshBottomFootR, 15, FColor::Cyan);

	// foot ik main locations
	// DrawDebugSphere(GetWorld(), SFCharacter->GetMesh()->GetSocketLocation(RootBoneName), 6, 4, FColor::Red);
	DrawDebugSphere(GetWorld(), MeshBottomFootR, 6, 4, FColor::Orange);
	DrawDebugSphere(GetWorld(), MeshBottomFootL, 6, 4, FColor::Orange);
	DrawDebugSphere(GetWorld(), MeshBottomFootR_RootZ, 3, 4, FColor::Yellow, false, -1.f, 0);
	DrawDebugSphere(GetWorld(), MeshBottomFootL_RootZ, 3, 4, FColor::Yellow, false, -1.f, 0);

	// IK Alpha values
	//DrawDebugString(GetWorld(), MeshBottomFootL, FString("AlphaL: ") + FString::SanitizeFloat(IKAlphaLegLeft), 0, FColor::White, 0.f, true);
	//DrawDebugString(GetWorld(), MeshBottomFootR, FString("AlphaR: ") + FString::SanitizeFloat(IKAlphaLegRight), 0, FColor::White, 0.f, true);
	DrawDebugString(GetWorld(), SFCharacter->GetMesh()->GetSocketLocation(HipBoneName), FString("HipZDisplacement: ") + FString::SanitizeFloat(IKLegHipDisplacementZ), 0, FColor::White, 0.f, true);
}
