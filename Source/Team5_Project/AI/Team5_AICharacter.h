#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Team5_AICharacter.generated.h"

class UTeam5_DamageTakenComponent;

UCLASS()
class TEAM5_PROJECT_API ATeam5_AICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATeam5_AICharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageComp")
	UTeam5_DamageTakenComponent* DamageTakenComponent;
};
