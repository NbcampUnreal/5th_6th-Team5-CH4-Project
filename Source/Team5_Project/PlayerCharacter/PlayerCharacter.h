// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class TEAM5_PROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

private:
	UPROPERTY(VisibleAnywhere, Category = "Input")
	class UInputMappingContext* DefaultContext;

	UPROPERTY(VisibleAnywhere, Category = "Input")
	class UInputAction* MoveAction;

	UPROPERTY(VisibleAnywhere, Category = "Input")
	class UInputAction* LookAction;

	UPROPERTY(VisibleAnywhere, Category = "Input")
	class UInputAction* JumpAction;

	UPROPERTY(VisibleAnywhere, Category = "Input")
	class UInputAction* AttackAction;

	UPROPERTY(VisibleAnywhere, Category = "Input")
	float mouseSpeed = 30;

	UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bIsFalling = false;

	UFUNCTION()
	void Attack(const struct FInputActionValue& Value);

	bool DoLineTrace(FHitResult& OutHit) const;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceDistance = 150.f;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
private:
	//에디터에서 선택할 수 있도록 설정
	UPROPERTY(EditDefaultsOnly, Category = "Axe")
	TSubclassOf<class AAxe> AxeClass;
	
	UPROPERTY()
	AAxe* Axe = nullptr;
};