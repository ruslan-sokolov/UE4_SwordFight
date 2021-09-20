// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/SFWeapon.h"
#include "Components/SkeletalMeshComponent.h"

#include <SwordFight/Public/Player/SFCharacter.h>

// Sets default values
ASFWeapon::ASFWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkMesh"));
	RootComponent = MeshComp;

}

// Called when the game starts or when spawned
void ASFWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASFWeapon::OnAttachToCharacter(ASFCharacter* CharacterToAttach, EWeaponEquipHand PreferredHand)
{
	OwnedCharacter = CharacterToAttach;
	SetOwner(CharacterToAttach);
	bCharacterHandIsLeft = PreferredHand == EWeaponEquipHand::LeftHand;
}

void ASFWeapon::Attack()
{
}

void ASFWeapon::StartBlock()
{
}

void ASFWeapon::StopBlock()
{
}
