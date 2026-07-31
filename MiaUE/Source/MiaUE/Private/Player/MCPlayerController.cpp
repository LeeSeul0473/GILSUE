// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MCPlayerController.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"

void AMCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}
