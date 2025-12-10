#include "AI/Team5_AISpawner.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

ATeam5_AISpawner::ATeam5_AISpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	MaxSpawnAI = 10;
	SpawnRadius = 500.f;

	//스피어 컴포넌트를 루트로 만들고 범위를 만들고 충돌 나지 않게 함
	SpawnRadiusComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SpawnRadiusComponent"));
	if (SpawnRadiusComponent)
	{
		RootComponent = SpawnRadiusComponent;
		SpawnRadiusComponent->SetSphereRadius(SpawnRadius);
		SpawnRadiusComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ATeam5_AISpawner::BeginPlay()
{
	Super::BeginPlay();

	//게임 시작 후 한번 실행
	SpawnAI();
}

void ATeam5_AISpawner::SpawnAI()
{
	//에디터에서 AIClassSpawn이 설정되어 있지 않으면 소환하지 않음
	if (!AIClassSpawn)
	{
		return;
	}
	//월드가 있는지 확인
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	//스포너 액터의 위치를 가지고옴
	FVector OriginLocation = GetActorLocation();
	//저장한 수 만큼 AI소환
	for (int32 i = 0; i < MaxSpawnAI; ++i)
	{
		//Spawner의 범위 내에서 랜덤으로 소환
		FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.0f, SpawnRadius);
		FVector SpawnLocation = OriginLocation + RandomOffset;
		//AI의 초기 회전값(0,0,0)
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		//충돌 시 조정하여 스폰
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (ACharacter* NewAI = World->SpawnActor<ACharacter>(AIClassSpawn, SpawnLocation, SpawnRotation, SpawnParams))
		{
			UE_LOG(LogTemp, Error, TEXT("AMonsterSpawner: AI 스폰 성공! 위치: %s"), *NewAI->GetActorLocation().ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AMonsterSpawner: %d번째 AI 스폰 실패!"), i);
		}
	}
}
