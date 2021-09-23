// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/SFWeapon.h"
#include "Components/SkeletalMeshComponent.h"

#include <SwordFight/Public/Player/SFCharacter.h>

#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"

// Sets default values
ASFWeapon::ASFWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkMesh"));
	RootComponent = MeshComp;

	BoxCollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollisionComp->SetupAttachment(MeshComp);
	BoxCollisionComp->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
}

// Called when the game starts or when spawned
void ASFWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASFWeapon::OnAttachToCharacter(ASFCharacter* CharacterToAttach, EWeaponEquipHand PreferredHand)
{
	if (CharacterToAttach == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASFWeapon::OnAttachToCharacter() for %s failed, Character is NULL"), *this->GetName());
		return;
	}

	OwnedCharacter = CharacterToAttach;
	SetOwner(CharacterToAttach);
	bCharacterHandIsLeft = PreferredHand == EWeaponEquipHand::LeftHand;

	// ignore collision weapon->owner owner->weapon
	BoxCollisionComp->MoveIgnoreActors.Add(CharacterToAttach);
	CharacterToAttach->MoveIgnoreActorAdd(this);

	// call blueprint event
	OnAttached(CharacterToAttach, PreferredHand);
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
