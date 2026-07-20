// Fill out your copyright notice in the Description page of Project Settings.


#include "LSRocket.h"
#include "../MiaBP.h"

// Sets default values
ALSRocket::ALSRocket()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALSRocket::BeginPlay()
{
	Super::BeginPlay();
	
	OnActorBeginOverlap.AddDynamic(this, &ALSRocket::ProcessActorBeginOverlap);
}

// Called every frame
void ALSRocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALSRocket::ProcessActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (OverlappedActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("BeginOverlap %s"), *OtherActor->GetName());
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionTemplate,
		GetActorLocation());
	UGameplayStatics::SpawnSound2D(GetWorld(), ExplosionSound);

	UGameplayStatics::ApplyDamage(OtherActor,
		10,
		UGameplayStatics::GetPlayerController(GetWorld(), 0),
		this,
		nullptr
	);

	Destroy();
}

