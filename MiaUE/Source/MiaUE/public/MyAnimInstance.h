// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MIAUE_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:

    virtual void NativeUpdateAnimation(float DeltaSeconds) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float Direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	uint32 bArmed : 1 = false;
};
