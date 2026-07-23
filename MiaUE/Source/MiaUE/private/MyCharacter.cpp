// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
		FRotator(0, -90, 0));

	NewTargetArmLength = CameraBoom->TargetArmLength;

}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, NewTargetArmLength, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()),
		4.0f);

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* UIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (UIC)
	{
		UIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AMyCharacter::Jump);
		UIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
		UIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMyCharacter::Move);

		UIC->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &AMyCharacter::Zoom);
	}

}

void AMyCharacter::Look(const FInputActionValue& Value)
{
	FVector2D Direction = Value.Get<FVector2D>();

	AddControllerPitchInput(Direction.Y);
	AddControllerYawInput(Direction.X);
//	AddControllerRollInput(Direction.Y);
}

void AMyCharacter::Move(const FInputActionValue& Value)
{
	FVector2D Direction = Value.Get<FVector2D>();

	FRotator CameraRotaion = GetControlRotation();

	FVector ForawrdVector =  UKismetMathLibrary::GetForwardVector( FRotator(0, CameraRotaion.Yaw, 0));
	FVector RightVector =  UKismetMathLibrary::GetRightVector(FRotator(0, CameraRotaion.Yaw, CameraRotaion.Roll));

	
	AddMovementInput(ForawrdVector * Direction.X);
	AddMovementInput(RightVector * Direction.Y);
}

void AMyCharacter::Zoom(const FInputActionValue& Value)
{
	NewTargetArmLength = CameraBoom->TargetArmLength - Value.Get<float>() * 700.0f;
	NewTargetArmLength = FMath::Clamp(NewTargetArmLength, 40, NewTargetArmLength);
}



