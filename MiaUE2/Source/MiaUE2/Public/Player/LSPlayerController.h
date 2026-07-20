// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MIAUE2_API ALSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Input")
	TSoftObjectPtr<class UInputMappingContext> InputMapping;
};
