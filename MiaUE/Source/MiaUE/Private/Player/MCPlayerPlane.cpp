// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MCPlayerPlane.h"
#include "MCLOG.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
//#include "EnhancedInputSubsystems.h"


AMCPlayerPlane::AMCPlayerPlane()
{
	//PrimaryActorTick.bCanEverTick = true;

	BoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RootComponent = BoxCollisionComponent;

	BodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMeshComponent->SetupAttachment(RootComponent);

	LeftMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftMesh"));
	LeftMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Left"));

	RightMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightMesh"));
	RightMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Right"));

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(BoxCollisionComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	FloatingMovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));

	//static ConstructorHelpers::FClassFinder<AMyRocket> ClassPath(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/BP_Rocket.BP_Rocket_C'"));
	//if (ClassPath.Succeeded())
	//{
	//	RocketTemplate = ClassPath.Class;
	//}
}

void AMCPlayerPlane::BeginPlay()
{
	Super::BeginPlay();

	MC_LOG(LogMC, Log, TEXT("Begin"));
	
}

// Called every frame
//void AMCPlayerPlane::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

// Called to bind functionality to input
void AMCPlayerPlane::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AMCPlayerPlane::Fire);
	EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Triggered, this, &AMCPlayerPlane::Rotate);
}

void AMCPlayerPlane::Fire()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));
}

void AMCPlayerPlane::Rotate(const FInputActionValue& Value)
{
	FVector2D RotateVector = Value.Get<FVector2D>();

	MC_LOG(LogMC, Log, TEXT("Begin : %f, %f"), RotateVector.X, RotateVector.Y);
}


