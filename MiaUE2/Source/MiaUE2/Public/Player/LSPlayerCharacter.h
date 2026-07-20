// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LSPlayerCharacter.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;

UCLASS()
class MIAUE2_API ALSPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALSPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

};
