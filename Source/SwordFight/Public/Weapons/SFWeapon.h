// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SFWeapon.generated.h"

class ASFCharacter;
class UBoxComponent;

// todo: enums to separate file

UENUM(BlueprintType)
enum class EWeaponEquipHand : uint8 {
	LeftHand = 0    UMETA(DisplayName = "Left Hand"),
	RightHand = 1   UMETA(DisplayName = "Right Hand"),
	Any = 2         UMETA(DisplayName = "Any Hand"),
};

UENUM(BlueprintType)
enum class EWeaponUseCapability : uint8 {
	BlockOnly = 0   UMETA(DisplayName = "Block Only"),
	AttackOnly = 1  UMETA(DisplayName = "Attack Only"),
	Any = 2	        UMETA(DisplayName = "Block And Attack"),
};

UENUM(BlueprintType)
enum class EWeaponHoldType : uint8 {
	OneHanded = 0   UMETA(DisplayName = "One Handed"),
	TwoHanded = 1  UMETA(DisplayName = "Two Handed"),
};

UENUM(BlueprintType)
enum class EWeaponAttackType : uint8 {
	Melee = 0   UMETA(DisplayName = "Melee"),
	Range = 1  UMETA(DisplayName = "Range"),
};

/**
 * Basic weapon class, axes are weapons, shields are weapons, swords are weapons
 */
UCLASS()
class SWORDFIGHT_API ASFWeapon : public AActor
{
	GENERATED_BODY()

protected:
	/** Weapon mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	USkeletalMeshComponent* MeshComp;

	/** Weapon mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UBoxComponent* BoxCollisionComp;

	/* To which hand weapon can be attached */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponEquipHand EquipHand;

	/** Can weapon attack, block or both */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponUseCapability UseCapatibility;

	/** Is weapon OneHanded or TwoHanded */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponHoldType HoldType;

	/** Is melee weapon or range weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponAttackType AttackType;

	/** Base weapon damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "UseCapatibility >= 1"))
	float BaseDamage;

	/** Weapon block effectivenes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "UseCapatibility == 0 || UseCapatibility == 2"))
	float BlockEffectiveness;


	/** Current character attached to */
	UPROPERTY(BlueprintReadOnly, Category = "weapon", meta = (AllowPrivateAccess = "true"))
	ASFCharacter* OwnedCharacter;

	/** character attached to hand */
	UPROPERTY(BlueprintReadOnly, Category = "weapon", meta = (AllowPrivateAccess = "true"))
	bool bCharacterHandIsLeft;


	/** is attack in progress */
	UPROPERTY()
	bool bIsAttacking;

	/** is blocking now */
	UPROPERTY()
	bool bIsBlocking;

public:	
	// Sets default values for this actor's properties
	ASFWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// called when OnAttachToCharacter method is used in code
	UFUNCTION(BlueprintImplementableEvent)
	void OnAttached(ASFCharacter* CharacterToAttach, EWeaponEquipHand PreferredHand);

public:	
	
	virtual void OnAttachToCharacter(ASFCharacter* CharacterToAttach, EWeaponEquipHand PreferredHand = EWeaponEquipHand::Any);

	virtual void Attack();

	virtual void StartBlock();
	virtual void StopBlock();

	/** is attack in progress */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapon")
	FORCEINLINE bool IsAttacking() const { return bIsAttacking; }

	/** is blocking now */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapon")
	FORCEINLINE bool IsBlocking() const { return bIsBlocking; }

};
