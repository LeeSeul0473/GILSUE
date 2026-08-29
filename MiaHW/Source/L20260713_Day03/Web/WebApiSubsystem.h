// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "WebApiSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWebApiResultSignature, const bool, bInSuccess, const FString&, InMessage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FWebApiServerInfoSignature,
	const bool, bInSuccess, const FString&, InServerIP, const int32, InServerPort, const FString&, InMessage);

/**
 * 웹서버와의 HTTP 통신을 전담한다. 결과는 델리게이트로만 알린다.
 */
UCLASS()
class L20260713_DAY03_API UWebApiSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "WebApi")
	FWebApiResultSignature OnLoginResult;

	UPROPERTY(BlueprintAssignable, Category = "WebApi")
	FWebApiResultSignature OnSignUpResult;

	UPROPERTY(BlueprintAssignable, Category = "WebApi")
	FWebApiResultSignature OnRegisterServerResult;

	UPROPERTY(BlueprintAssignable, Category = "WebApi")
	FWebApiServerInfoSignature OnServerInfoResult;

	void RequestLogin(const FString& InServerIP, const FString& InUserID, const FString& InPassword);

	void RequestSignUp(const FString& InServerIP, const FString& InUserID, const FString& InPassword);

	void RequestRegisterServer(const FString& InWebServerIP, int32 InGamePort);

	void RequestServerInfo(const FString& InWebServerIP);

private:

	void SendAuthRequest(const FString& InServerIP, const FString& InPath,
		const FString& InUserID, const FString& InPassword,
		FWebApiResultSignature& InDelegate, const bool bInIsLogin);

	void HandleAuthResponse(FHttpResponsePtr InResponse, const bool bInConnectedSuccessfully,
		FWebApiResultSignature& InDelegate, const bool bInIsLogin);

	void HandleRegisterServerResponse(FHttpResponsePtr InResponse, const bool bInConnectedSuccessfully);

	void HandleServerInfoResponse(FHttpResponsePtr InResponse, const bool bInConnectedSuccessfully);
};
