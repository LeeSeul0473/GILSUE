#pragma once

#include "CoreMinimal.h"

//StandAlone
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
#define MC_LOG(CatName, Verbosity, Format, ...) UE_LOG(CatName, Verbosity, TEXT("[%s] : %s"), LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

//Network
//#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client)? *FString::Printf(TEXT("CLIENT%d"), UE::GetPlayInEditorID()) : (GetNetMode() == ENetMode::NM_Standalone)? TEXT("STANDALONE") : TEXT("SERVER"))
//#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
//#define LOG_SUBLOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetOwner()->GetLocalRole()))
//#define LOG_SUBREMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetOwner()->GetRemoteRole()))
//#define LOG_WIDGET_NETMODEINFO (GetWorld() ? ((GetWorld()->GetNetMode() == ENetMode::NM_Client)? *FString::Printf(TEXT("CLIENT%d"), UE::GetPlayInEditorID()) : (GetWorld()->GetNetMode() == ENetMode::NM_Standalone)? TEXT("STANDALONE") : TEXT("SERVER")) : TEXT("UNKNOWN"))
//
//#define MC_LOG(CatName, Verbosity, Format, ...) UE_LOG(CatName, Verbosity, TEXT("[%s] %s : %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
//#define MC_SUBLOG(CatName, Verbosity, Format, ...) UE_LOG(CatName, Verbosity, TEXT("[%s][%s/%s] %s : %s"), LOG_NETMODEINFO, LOG_SUBLOCALROLEINFO, LOG_SUBREMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
//#define MC_WDGLOG(CatName, Verbosity, Format, ...) UE_LOG(CatName, Verbosity, TEXT("[%s] %s : %s"), LOG_WIDGET_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogMC, Log, All);
