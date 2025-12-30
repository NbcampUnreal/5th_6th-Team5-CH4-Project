#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Team5_DamageTakenComponent.generated.h"

// 방송(델리게이트)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TEAM5_PROJECT_API UTeam5_DamageTakenComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTeam5_DamageTakenComponent();

	// 외부에서 죽음 이벤트를 들을 수 있게
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeathDelegate OnDeathDelegate;

	// HP 읽기용 함수
	UFUNCTION(BlueprintCallable, Category = "HP")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintCallable, Category = "HP")
	float GetMaxHP() const { return MaxHP; }

	// HP도 복제되도록 변경
	UPROPERTY(ReplicatedUsing=OnRep_CurrentHP, EditAnywhere, BlueprintReadWrite, Category = "HP")
	float CurrentHP;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "HP")
	float MaxHP;

	UPROPERTY(ReplicatedUsing=OnRep_IsAlive, VisibleAnywhere, BlueprintReadOnly, Category = "HP")
	bool bIsAlive;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy,
					   AActor* DamageCauser);

	// CurrentHP 변경이 클라로 왔을 때 호출
	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_IsAlive();

	void DeathCollision(APawn* MyPawn);
};
