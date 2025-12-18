// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

// [추가] 컴포넌트 클래스 전방 선언 (헤더 참조 충돌 방지)
class UTeam5_DamageTakenComponent;

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

	// [추가] 서버로 공격 요청을 보내는 RPC 함수 (클라이언트 -> 서버)
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Attack(FVector Start, FVector Dir);
	
protected:
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	// [추가] 내 몸에 부착된 데미지 처리 컴포넌트를 저장할 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UTeam5_DamageTakenComponent* DamageComp;

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

	// [수정] 트레이스 함수: 서버에서도 궤적을 계산해야 하므로 시작점(Start)과 방향(Dir)을 인자로 받도록 변경
	bool DoLineTrace(FHitResult& OutHit, FVector Start, FVector Dir) const;
    
	// [추가] 컴포넌트가 "죽었다"고 신호를 보내면 실행될 함수
	UFUNCTION()
	void OnDeath();

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