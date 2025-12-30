#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "T5PlayerState.generated.h"

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
    Waiting,
    Hunter,
    Animal
};

UCLASS()
class TEAM5_PROJECT_API AT5PlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AT5PlayerState();

    UFUNCTION(BlueprintCallable)
    void SetPlayerRole(EPlayerRole NewRole);

    UFUNCTION(BlueprintPure)
    EPlayerRole GetPlayerRole() const { return CurrentRole; }

    // 서버에서만 호출되도록 운용(HP 감소/죽음 확정)
    UFUNCTION(BlueprintCallable)
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintPure)
    bool IsDead() const { return bIsDead; }

public:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentRole, BlueprintReadOnly)
    EPlayerRole CurrentRole;

    // HP 변수
    UPROPERTY(Replicated, BlueprintReadOnly)
    float CurrentHP;

    UPROPERTY(Replicated, BlueprintReadOnly)
    FName CharacterID;

    // 죽음 상태
    UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly)
    bool bIsDead;

protected:
    UFUNCTION()
    void OnRep_CurrentRole();

    UFUNCTION()
    void OnRep_IsDead();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
