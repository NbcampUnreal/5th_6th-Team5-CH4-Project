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
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable)
    void SetPlayerRole(EPlayerRole NewRole);

    UFUNCTION(BlueprintPure)
    EPlayerRole GetPlayerRole() const { return CurrentRole; }

    void ApplyDamage(float Amount);

public:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentRole, BlueprintReadOnly)
    EPlayerRole CurrentRole;

    UPROPERTY(Replicated, BlueprintReadOnly)
    float CurrentHP;

    // [추가] 룰북 검색을 위한 캐릭터 ID (Hunter, Animal 등)
    UPROPERTY(Replicated, BlueprintReadOnly)
    FName CharacterID;

protected:
    UFUNCTION()
    void OnRep_CurrentRole();
};