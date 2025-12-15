#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Team5_DamageTakenComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAM5_PROJECT_API UTeam5_DamageTakenComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeam5_DamageTakenComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(editAnywhere, BlueprintReadWrite, Category = "HP")
	float CurrentHP;
	UPROPERTY(editAnywhere, BlueprintReadWrite, Category = "HP")
	float MaxHP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HP")
	bool bIsAlive;

	UFUNCTION()
	void OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy,
					   AActor* DamageCauser);
};
