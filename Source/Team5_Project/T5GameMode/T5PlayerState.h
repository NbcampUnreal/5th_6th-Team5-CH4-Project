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

    void AddScore(float ScoreToAdd);
    void ApplyDamage(float Amount);

public:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentRole, BlueprintReadOnly)
    EPlayerRole CurrentRole;

    // [핵심] HP 변수
    UPROPERTY(Replicated, BlueprintReadOnly)
    float CurrentHP;

protected:
    UFUNCTION()
    void OnRep_CurrentRole();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};