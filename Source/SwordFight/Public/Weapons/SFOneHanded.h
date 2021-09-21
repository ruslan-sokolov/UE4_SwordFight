// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/SFWeapon.h"
#include "SFOneHanded.generated.h"

/**
 * 
 */
UCLASS()
class SWORDFIGHT_API ASFOneHanded : public ASFWeapon
{
	GENERATED_BODY()

public:

	ASFOneHanded();

	virtual void Attack() override;
	
};
