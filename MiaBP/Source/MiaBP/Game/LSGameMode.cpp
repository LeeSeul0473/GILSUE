// Fill out your copyright notice in the Description page of Project Settings.


#include "LSGameMode.h"
#include "../MiaBP.h"
#include "../Player/LSPlayerController.h"
#include "../Player/LSPlayerPawn.h"

ALSGameMode::ALSGameMode()
{
	//CDO 초기화할 때 사용
	LS_LOG(LogLS, Log, TEXT("Hello World!"));

	DefaultPawnClass = ALSPlayerPawn::StaticClass();
	PlayerControllerClass = ALSPlayerController::StaticClass();
}
