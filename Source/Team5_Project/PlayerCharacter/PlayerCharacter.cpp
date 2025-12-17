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

	SetActorRotation(NewRot);
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y * GetWorld()->DeltaRealTimeSeconds * mouseSpeed);
	AddControllerYawInput(LookAxisVector.X * GetWorld()->DeltaRealTimeSeconds * mouseSpeed);
}

bool APlayerCharacter::DoLineTrace(FHitResult& OutHit) const
{
	if (!Controller) return false;

	const FVector Start = GetMesh()->GetComponentLocation() + FVector(0.f, 0.f, 50.f);
	const FVector End = Start + GetActorForwardVector() * TraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(PlayerTrace), true);
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.5f);

	if (bHit)
	{
		DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 8.f, 12, FColor::Red, false, 1.0f);
	}

	return bHit;
}

void APlayerCharacter::Attack(const struct FInputActionValue& Value)
{
	// 1. 로그로 작동 확인
	UE_LOG(LogTemp, Warning, TEXT("공격 버튼 눌림!"));

	FHitResult Hit;
	if (DoLineTrace(Hit))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s | Point: %s"),
				*HitActor->GetName(),
				*Hit.ImpactPoint.ToString()
			);
		}
	}
	// 2. 공격 애니메이션 실행 (나중에 추가)
    
	// 3. 공격 판정 로직 (나중에 여기에 Raycast 등을 넣어서 GameMode->ProcessAttack 호출)
	
}