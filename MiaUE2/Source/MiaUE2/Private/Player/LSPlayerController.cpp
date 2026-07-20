// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LSPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"


void ALSPlayerController::BeginPlay()
{
    if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (!InputMapping.IsNull())
            {
                InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
            }
        }
    }
}
