#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Team5_DamageTakenComponent.generated.h"

// [추가] 방송을 위한 델리게이트 선언 매크로
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAM5_PROJECT_API UTeam5_DamageTakenComponent : public UActorComponent
{
    GENERATED_BODY()

public: 
    UTeam5_DamageTakenComponent();

    // [추가] 외부에서 이 방송을 들을 수 있게 변수 선언
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDeathDelegate OnDeathDelegate;

    // [추가] 현재 체력을 외부에서 확인하기 위한 함수
    UFUNCTION(BlueprintCallable, Category = "HP")
    float GetCurrentHP() const { return CurrentHP; }

    UPROPERTY(editAnywhere, BlueprintReadWrite, Category = "HP")
    float CurrentHP;
    UPROPERTY(editAnywhere, BlueprintReadWrite, Category = "HP")
    float MaxHP;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HP")
    bool bIsAlive;

protected:
    virtual void BeginPlay() override;

  

    UFUNCTION()
    void OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
};