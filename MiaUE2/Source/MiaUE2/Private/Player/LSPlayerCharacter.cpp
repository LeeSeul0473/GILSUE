// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LSPlayerCharacter.h"
#include "../MiaUE2.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALSPlayerCharacter::ALSPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
		FRotator(0, -90, 0));
}

// Called when the game starts or when spawned
void ALSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALSPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ALSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

