#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "MCPlayerPlane.generated.h"

UCLASS()
class MIAUE_API AMCPlayerPlane : public APawn
{
	GENERATED_BODY()

public:
	AMCPlayerPlane();

protected:
	virtual void BeginPlay() override;

public:	
	//virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components", BlueprintReadOnly)
	TObjectPtr<class UBoxComponent> BoxCollisionComponent;

	//Meshes
	UPROPERTY(VisibleAnywhere, Category = "Components", BlueprintReadOnly)
	TObjectPtr<class UStaticMeshComponent> BodyMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components", BlueprintReadOnly)
	TObjectPtr<class UStaticMeshComponent> LeftMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components", BlueprintReadOnly)
	TObjectPtr<class UStaticMeshComponent> RightMeshComponent;

	//Camera
	UPROPERTY(VisibleAnywhere, Category = "Components", BlueprintReadOnly)
	TObjectPtr<class USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components", BlueprintReadOnly)
	TObjectPtr<class UCameraComponent> CameraComponent;

	//Movement
	UPROPERTY(VisibleAnywhere, Category = "Components", BlueprintReadOnly)
	TObjectPtr<class UFloatingPawnMovement> FloatingMovementComponent;

	//Property
	UPROPERTY(VisibleAnywhere, Category = "Stat", BlueprintReadOnly)
	float Boost = 0.5f;

	//Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> RotateAction;

	//Rocket
	//UPROPERTY(EditAnywhere, Category = "Data", BlueprintReadWrite)
	//TSubclassOf<class AMyRocket> RocketTemplate;

protected:
	void Fire();
	void Rotate(const FInputActionValue& Value);

};
