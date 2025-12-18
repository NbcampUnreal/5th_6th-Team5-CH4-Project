// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include <Kismet/KismetMathLibrary.h>
#include "Axe.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "T5GameMode/T5GameMode.h"
#include "AI/Team5_DamageTakenComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;

	ConstructorHelpers::FObjectFinder<USkeletalMesh>PlayerMesh(TEXT("Script/Engine.SkeletalMesh'/Game/KNC_Chatacter/Character/Animations/Ch09_nonPBR.Ch09_nonPBR'"));

	if (PlayerMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(PlayerMesh.Object);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));
		GetMesh()->SetUsingAbsoluteRotation(false);
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	if (SpringArm)
	{
		SpringArm->SetupAttachment(RootComponent);
		SpringArm->SetWorldLocation(FVector(0, 0, 55));
		SpringArm->TargetArmLength = 150;
		SpringArm->SocketOffset = FVector(0, 50, 0);

		SpringArm->bUsePawnControlRotation = true;
	}
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	if (Camera)
	{
		Camera->SetupAttachment(SpringArm);
		Camera->bUsePawnControlRotation = false;
	}

	DamageComp = CreateDefaultSubobject<UTeam5_DamageTakenComponent>(TEXT("DamageTakenComponent"));

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputContext(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/KNC_Chatacter/Input/IMC_DefaultInput.IMC_DefaultInput'"));
	if (InputContext.Succeeded())
	{
		DefaultContext = InputContext.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputMove(TEXT("/Script/EnhancedInput.InputAction'/Game/KNC_Chatacter/Input/IA_Move.IA_Move'"));
	if (InputMove.Succeeded())
	{
		MoveAction = InputMove.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputLook(TEXT("/Script/EnhancedInput.InputAction'/Game/KNC_Chatacter/Input/IA_Look.IA_Look'"));
	if (InputLook.Succeeded())
	{
		LookAction = InputLook.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputJump(TEXT("/Script/EnhancedInput.InputAction'/Game/KNC_Chatacter/Input/IA_Jump.IA_Jump'"));
	if (InputJump.Succeeded())
	{
		JumpAction = InputJump.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputAttack(TEXT("/Script/EnhancedInput.InputAction'/Game/KNC_Chatacter/Input/IA_Attack.IA_Attack'"));
	if (InputAttack.Succeeded())
	{
		AttackAction = InputAttack.Object;
	}
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			SubSystem->AddMappingContext(DefaultContext, 0);
		}
	}

	auto* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->RotationRate = FRotator(0.f, 0.f, 0.f);
	}


	GetCharacterMovement()->MaxWalkSpeed = 600;

	DamageComp = FindComponentByClass<UTeam5_DamageTakenComponent>();
	if (DamageComp)
	{
		DamageComp->OnDeathDelegate.AddDynamic(this, &APlayerCharacter::OnDeath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("오류: 캐릭터에 DamageTakenComponent가 없습니다! 블루프린트에서 추가해주세요."));
	}

	//Axe를 가지고 와서 SkeletonMesh의 RightHandSocket에 붙히고 PlayerController에서 사용할 수 있도록 설정
	if (AxeClass)
	{
		Axe = GetWorld()->SpawnActor<AAxe>(AxeClass);

		if (Axe)
		{
			Axe->SetOwner(this);
			Axe->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("RightHandSocket"));
		}
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			Axe->OwnerController = PC;
		}
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetCharacterMovement())
	{
		bIsFalling = GetCharacterMovement()->IsFalling();
	}

	if (bIsAttacking && Controller)
	{
		FRotator TargetRot(0.f, AttackLockedYaw, 0.f);

		FRotator NewRot = FMath::RInterpTo(
			GetActorRotation(),
			TargetRot,
			DeltaTime,
			AttackTurnSpeed
		);

		SetActorRotation(NewRot);
	}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Attack);
	
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller) return;

	const FVector2D Movement = Value.Get<FVector2D>();
	if (Movement.IsNearlyZero()) return;

	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator YawOnly(0.f, ControlRot.Yaw, 0.f);

	const FVector Forward = UKismetMathLibrary::GetForwardVector(YawOnly);
	const FVector Right = UKismetMathLibrary::GetRightVector(YawOnly);

	AddMovementInput(Forward, Movement.Y);
	AddMovementInput(Right, Movement.X);

	const FVector MoveDir = (Forward * Movement.Y + Right * Movement.X).GetSafeNormal();

	float TargetYaw = MoveDir.Rotation().Yaw;

	const FRotator CurrentRot = GetActorRotation();
	const FRotator TargetRot(0.f, TargetYaw, 0.f);

	const FRotator NewRot = FMath::RInterpTo(
		CurrentRot,
		TargetRot,
		GetWorld()->GetDeltaSeconds(),
		12.f
	);

	if (!bIsAttacking)
	{
		SetActorRotation(NewRot);
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y * GetWorld()->DeltaRealTimeSeconds * mouseSpeed);
	AddControllerYawInput(LookAxisVector.X * GetWorld()->DeltaRealTimeSeconds * mouseSpeed);
}

void APlayerCharacter::Attack(const struct FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("공격 버튼 눌림!"));

	if (Controller)
	{
		FVector Start, Dir;
		FRotator Rot;
		Controller->GetPlayerViewPoint(Start, Rot);
		Dir = Rot.Vector();

		Server_Attack(Start, Dir);
	}
}

bool APlayerCharacter::Server_Attack_Validate(FVector Start, FVector Dir)
{
	return true;
}

void APlayerCharacter::Server_Attack_Implementation(FVector Start, FVector Dir)
{
	FHitResult Hit;

	if (DoLineTrace(Hit, Start, Dir))
	{
		AActor* HitActor = Hit.GetActor();

		if (AT5GameMode* GM = Cast<AT5GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->ProcessAttack(GetController(), HitActor);
		}
	}
	else
	{
		if (AT5GameMode* GM = Cast<AT5GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->ProcessAttack(GetController(), nullptr);
		}
	}
}

bool APlayerCharacter::DoLineTrace(FHitResult& OutHit, FVector Start, FVector Dir) const
{
	const FVector End = Start + (Dir * TraceDistance);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(PlayerTrace), true);
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit, Start, End, ECC_Visibility, Params
	);

	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.5f);

	return bHit;
}

void APlayerCharacter::OnDeath()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (AT5GameMode* GM = Cast<AT5GameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ProcessActorDeath(this, nullptr);
	}
}