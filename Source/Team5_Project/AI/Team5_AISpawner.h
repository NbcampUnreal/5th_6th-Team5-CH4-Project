#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Team5_AISpawner.generated.h"

class USphereComponent;

UCLASS()
class TEAM5_PROJECT_API ATeam5_AISpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ATeam5_AISpawner();

protected:
	virtual void BeginPlay() override;
	
	//스폰할 BP클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<ACharacter> AIClassSpawn;
	
	//스폰할 AI숫자
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 MaxSpawnAI;
	
	//스포너의 위치를 중심으로 AI가 소환될 랜덤 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = "0.0"))
	float SpawnRadius;
	
	//에디터에서 스폰 반경을 시각적으로 확인할 수 있는 스피어 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	USphereComponent* SpawnRadiusComponent;
	
private:
	//실제 스폰 로직
	void SpawnAI();
};
