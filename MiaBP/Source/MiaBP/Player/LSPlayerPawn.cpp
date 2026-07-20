// Fill out your copyright notice in the Description page of Project Settings.


#include "LSPlayerPawn.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "../MiaBP.h"
#include "../Rocket/LSRocket.h"

// Sets default values
ALSPlayerPawn::ALSPlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = Box;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);

	Left = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left"));
	Left->SetupAttachment(Body);
	Left->SetWorldLocation(FVector(38.000000,21.000000,0.000000));

	Right = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right"));
	Right->SetupAttachment(Body);
	Left->SetWorldLocation(FVector(38.000000, -22.000000, 0.000000));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));

	static ConstructorHelpers::FClassFinder<APawn> RocketTemplateClassRef(TEXT("/Script/Engine.Blueprint'/Game/P38/Blueprints/BP_Rocket.BP_Rocket_C'"));
	if (RocketTemplateClassRef.Succeeded())
	{
		RocketTemplate = RocketTemplateClassRef.Class;
	}
}

// Called when the game starts or when spawned
void ALSPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALSPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Left->AddLocalRotation(FRotator(0, 0, 1440.0f * DeltaTime));
	Right->AddLocalRotation(FRotator(0, 0, 1440.0f * DeltaTime));
}

// Called to bind functionality to input
void ALSPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Fire"), EInputEvent::IE_Pressed, this, &ALSPlayerPawn::Fire);
	PlayerInputComponent->BindAxis(TEXT("Roll"), this, &ALSPlayerPawn::Roll);
	PlayerInputComponent->BindAxis(TEXT("Pitch"), this, &ALSPlayerPawn::Pitch);


}

void ALSPlayerPawn::CallBlueprint(int Money, FString Name)
{
	LS_LOG(LogTemp, Log, TEXT("%s got %d!"), *Name, Money);
}

void ALSPlayerPawn::Fire()
{
	LS_LOG(LogTemp, Log, TEXT("Fire!"));

	GetWorld()->SpawnActor<ALSRocket>(RocketTemplate, Body->GetSocketTransform(TEXT("RocketSpawn")));
}

void ALSPlayerPawn::Roll(float Value)
{	
	AddActorLocalRotation(UGameplayStatics::GetWorldDeltaSeconds(GetWorld()) * FRotator(0, Value * 60, 0));
}

void ALSPlayerPawn::Pitch(float Value)
{
	AddActorLocalRotation(UGameplayStatics::GetWorldDeltaSeconds(GetWorld()) * FRotator(Value * 60, 0, 0));
}

