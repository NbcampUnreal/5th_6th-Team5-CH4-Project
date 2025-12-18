// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "T5PlayerState.h"
#include "InputActionValue.h"
#include "Engine/EngineTypes.h"
#include "TimerManager.h"
#include "Animation/AnimMontage.h"
#include "PlayerCharacter.generated.h"

class UTeam5_DamageTakenComponent;

UCLASS()
class TEAM5_PROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	void UpdateCharacterMesh(EPlayerRole NewRole);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Appearance")
	USkeletalMesh* HunterMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Appearance")
	TSubclassOf<UAnimInstance> HunterAnimBP;

	UPROPERTY(EditDefaultsOnly, Category = "Appearance")
	USkeletalMesh* AnimalMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Appearance")
	TSubclassOf<UAnimInstance> AnimalAnimBP;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Attack(FVector Start, FVector Dir);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageComp")
	UTeam5_DamageTakenComponent* DamageTakenComponent;

protected:
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UTeam5_DamageTakenComponent* DamageComp;*/

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

	bool DoLineTrace(FHitResult& OutHit, FVector Start, FVector Dir) const;

	UFUNCTION()
	void OnDeath();

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceDistance = 50.f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackTraceDelay = 0.22f;

	FTimerHandle AttackTraceTimerHandle;

	bool bIsAttacking = false;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackTurnSpeed = 15.f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackFaceHoldTime = 0.25f;

	UPROPERTY(VisibleAnywhere, Category = "Attack")
	float AttackLockedYaw = 0.f;

	UFUNCTION()
	void Attack(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	AActor* LastHitActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	FHitResult LastHitResult;

	FTimerHandle AttackFaceOffTimerHandle;
	FVector CachedAttackStartLoc = FVector::ZeroVector;
	FVector CachedAttackShootDir = FVector::ForwardVector;

	bool DoLineTraceFromAxe(FHitResult& OutHit) const;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FName AttackStartNotifyName = "AttackStart";

	// 몽타주(에디터에서 지정)
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	UAnimMontage* AttackMontage = nullptr;

	// (선택) 타이밍 캐시로 쓸 거면 유지
	float AttackStartTime = -1.f;
	bool bAttackStartFired = false;

	UFUNCTION()
	void HandleAttackStartNotify();
	
private:
	//에디터에서 선택할 수 있도록 설정
	UPROPERTY(EditDefaultsOnly, Category = "Axe")
	TSubclassOf<class AAxe> AxeClass;
	
	UPROPERTY()
	AAxe* Axe = nullptr;
};