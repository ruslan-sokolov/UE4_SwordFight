// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/SFWeapon.h"
#include "SFShield.generated.h"

/**
 * 
 */
UCLASS()
class SWORDFIGHT_API ASFShield : public ASFWeapon
{
	GENERATED_BODY()
	
public:
	ASFShield();

	virtual void StartBlock() override;
	virtual void StopBlock() override;
};
