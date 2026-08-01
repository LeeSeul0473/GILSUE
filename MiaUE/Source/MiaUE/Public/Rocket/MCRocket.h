// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCRocket.generated.h"

UCLASS()
class MIAUE_API AMCRocket : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMCRocket();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components", BlueprintReadOnly)
	TObjectPtr<class UBoxComponent> BoxCollisionComponent;

	//Meshes
	UPROPERTY(VisibleAnywhere, Category = "Components", BlueprintReadOnly)
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<class UNiagaraSystem> ExplosionEffectTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<USoundBase> ExplosionSound;

	UFUNCTION()
	void ProcessActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION(BlueprintImplementableEvent)
	void CallCPPExecuteBP();

	UFUNCTION(BlueprintNativeEvent)
	void CallCPPExecuteBPDefault();

	void CallCPPExecuteBPDefault_Implementation();
};
