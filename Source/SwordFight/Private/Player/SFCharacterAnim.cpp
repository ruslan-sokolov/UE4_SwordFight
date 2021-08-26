// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SFCharacterAnim.h"

#include "Player/SFCharacter.h"
#include "DrawDebugHelpers.h"

USFCharacterAnim::USFCharacterAnim()
{
	// defaults
	IKFootR = FName("Foot_R");
	IKFootL = FName("Foot_L");

	FootBoneZOffset = 13.5f;
}

void USFCharacterAnim::NativeInitializeAnimation()
{
	SFCharacter = Cast<ASFCharacter>(TryGetPawnOwner());
}

void USFCharacterAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	if (SFCharacter == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SFCharacterAnim SFCharacter is nullptr! Please use this anim with SFCharacter as Character!"));
		return;
	}

	CalcMeshBottomFootPositions();

	DrawDebugFootIK();
}

FORCEINLINE void USFCharacterAnim::CalcMeshBottomFootPositions()
{
	const FVector BoneFootLocation_R = SFCharacter->GetMesh()->GetSocketLocation(IKFootR);
	const FVector BoneFootLocation_L = SFCharacter->GetMesh()->GetSocketLocation(IKFootL);
	
	const float BaseOffsetZ = FootBoneZOffset * SFCharacter->GetMesh()->GetComponentScale().Z;
	const FVector FootBottomOffset_R = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootR).GetForwardVector() * BaseOffsetZ;
	const FVector FootBottomOffset_L = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootL).GetForwardVector() * BaseOffsetZ;

	MeshBottomFootR = BoneFootLocation_R + FootBottomOffset_R;  // wtf???
	MeshBottomFootL = BoneFootLocation_L - FootBottomOffset_L;

	//DrawDebugSphere(GetWorld(), MeshBottomFootL, 6, 36, FColor::Red);
	//DrawDebugSphere(GetWorld(), MeshBottomFootR, 6, 36, FColor::Red);
}

FORCEINLINE void USFCharacterAnim::DrawDebugFootIK() const
{
	const FVector IkFootL_Normal = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootL).GetForwardVector();
	const FVector IkFootR_Normal = SFCharacter->GetMesh()->GetSocketQuaternion(IKFootR).GetForwardVector();

	FPlane PlaneL = FPlane(MeshBottomFootL, IkFootL_Normal);
	FPlane PlaneR = FPlane(MeshBottomFootR, IkFootR_Normal);

	DrawDebugSolidPlane(GetWorld(), PlaneL, MeshBottomFootL, 15, FColor::Cyan);
	DrawDebugSolidPlane(GetWorld(), PlaneR, MeshBottomFootR, 15, FColor::Cyan);
}
