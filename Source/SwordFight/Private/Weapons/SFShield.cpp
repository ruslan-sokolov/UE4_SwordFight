// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/SFShield.h"

ASFShield::ASFShield()
{

	// defaults
	EquipHand = EWeaponEquipHand::LeftHand;
	UseCapatibility = EWeaponUseCapability::BlockOnly;
	HoldType = EWeaponHoldType::OneHanded;

	BlockEffectiveness = 0.75f;
}

void ASFShield::StartBlock()
{
	bIsBlocking = true;
}

void ASFShield::StopBlock()
{
	bIsBlocking = false;
}