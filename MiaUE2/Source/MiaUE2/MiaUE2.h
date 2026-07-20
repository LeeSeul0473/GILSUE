// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
#define LS_LOG(CatName, Verbosity, Format, ...) UE_LOG(CatName, Verbosity, TEXT("[%s] : %s"), LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
DECLARE_LOG_CATEGORY_EXTERN(LogLS, Log, All);