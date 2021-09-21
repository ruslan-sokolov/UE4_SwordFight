// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/SFOneHanded.h"

ASFOneHanded::ASFOneHanded()
{
	// defaults
	EquipHand = EWeaponEquipHand::RightHand;
	UseCapatibility = EWeaponUseCapability::AttackOnly;
	HoldType = EWeaponHoldType::OneHanded;
	AttackType = EWeaponAttackType::Melee;

	BaseDamage = 25.f;
}

void ASFOneHanded::Attack()
{
	bIsAttacking = true;

	FTimerHandle TimerHandle_Empty;
	GetWorldTimerManager().SetTimer(TimerHandle_Empty, [&]() { bIsAttacking = false; }, 1.f, false);
}