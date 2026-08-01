// Fill out your copyright notice in the Description page of Project Settings.


#include "Rocket/MCRocket.h"
#include "MCLOG.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMCRocket::AMCRocket()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	BoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RootComponent = BoxCollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMCRocket::BeginPlay()
{
	Super::BeginPlay();

	MC_LOG(LogMC, Log, TEXT("Begin"));

	OnActorBeginOverlap.AddDynamic(this, &AMCRocket::ProcessActorBeginOverlap);
}

// Called every frame
void AMCRocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMCRocket::ProcessActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (OtherActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}

	MC_LOG(LogMC, Log, TEXT("Actor Overlap : %s"), *OtherActor->GetName());

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionEffectTemplate, GetActorLocation());

	UGameplayStatics::SpawnSound2D(GetWorld(), ExplosionSound);

	UGameplayStatics::ApplyDamage(OtherActor, 10, UGameplayStatics::GetPlayerController(GetWorld(), 0), this, nullptr);

	Destroy();
}

void AMCRocket::CallCPPExecuteBPDefault_Implementation()
{

}

