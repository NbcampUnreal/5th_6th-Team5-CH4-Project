#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Axe.generated.h"

UCLASS()
class TEAM5_PROJECT_API AAxe : public AActor
{
	GENERATED_BODY()
	
public:	
	AAxe();

protected:
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(VisibleAnywhere, Category = "Axe")
	UStaticMeshComponent* Axe_Mesh = nullptr;
	
	UPROPERTY()
	APlayerController* OwnerController = nullptr;
};
