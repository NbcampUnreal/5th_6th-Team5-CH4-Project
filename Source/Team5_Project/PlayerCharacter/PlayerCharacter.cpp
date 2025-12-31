// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "T5PlayerState.h"
#include <Kismet/KismetMathLibrary.h>
#include "Axe.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "T5GameMode/T5GameMode.h"
#include "AI/Team5_DamageTakenComponent.h"
#include "Components/CapsuleComponent.h"
#include "PlayerCharacter/PlayerCharacterAnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	bUseControllerRotationYaw = false;
	

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

	DamageTakenComponent = CreateDefaultSubobject<UTeam5_DamageTakenComponent>("DamageTakenComponent");

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

	if (UPlayerCharacterAnimInstance* Anim = Cast<UPlayerCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->OnAttackStart.RemoveAll(this);
		Anim->OnAttackStart.AddUObject(this, &APlayerCharacter::HandleAttackStartNotify);

		Anim->OnSFX_Player_Footstep1.RemoveAll(this);
		Anim->OnSFX_Player_Footstep1.AddUObject(this, &APlayerCharacter::HandleFootstepLeft);

		Anim->OnSFX_Player_Footstep2.RemoveAll(this);
		Anim->OnSFX_Player_Footstep2.AddUObject(this, &APlayerCharacter::HandleFootstepRight);
	}

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
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->RotationRate = FRotator(0.f, 500.f, 0.f);
	}


	GetCharacterMovement()->MaxWalkSpeed = 600;

	DamageTakenComponent = FindComponentByClass<UTeam5_DamageTakenComponent>();
	if (DamageTakenComponent)
	{
		DamageTakenComponent->OnDeathDelegate.AddDynamic(this, &APlayerCharacter::OnDeath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("오류: 캐릭터에 DamageTakenComponent가 없습니다! 블루프린트에서 추가해주세요."));
	}

	//Axe를 가지고 와서 SkeletonMesh의 RightHandSocket에 붙히고 PlayerController에서 사용할 수 있도록 설정
	if (HasAuthority() && AxeClass)
	{
		Axe = GetWorld()->SpawnActor<AAxe>(AxeClass);
		if (Axe)
		{
			Axe->SetOwner(this);
			Axe->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("RightHandSocket"));
		}
	}

	// 서버는 이미 붙였지만, 리슨서버/싱글에서도 OnRep 호출 안 될 수 있어서 안전하게 한 번 호출
	if (!HasAuthority())
	{
		// 클라는 복제 받은 뒤 OnRep_Axe가 자동으로 호출됨
	}
	else
	{
		OnRep_Axe(); // 서버에서도 OwnerController 세팅 같은 거 해주고 싶으면
	}

	if (AT5PlayerState* PS = GetPlayerState<AT5PlayerState>())
	{
		UpdateCharacterMesh(PS->CurrentRole);
	}

}

void APlayerCharacter::UpdateCharacterMesh(EPlayerRole NewRole)
{
	USkeletalMesh* NewMesh = nullptr;
	TSubclassOf<UAnimInstance> NewAnimBP = nullptr;
	bool bShowWeapon = false;

	switch (NewRole)
	{
	case EPlayerRole::Hunter:
		NewMesh = HunterMesh;
		NewAnimBP = HunterAnimBP;
		bShowWeapon = true;
		break;

	case EPlayerRole::Animal:
		NewMesh = AnimalMesh;
		NewAnimBP = AnimalAnimBP;
		bShowWeapon = false;
		break;

	default:
		return;
	}

	if (NewMesh && GetMesh())
	{
		GetMesh()->SetSkeletalMesh(NewMesh);

		// GetMesh()->SetRelativeLocation(FVector(0, 0, -90)); 
		// GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));;
	}

	if (NewAnimBP && GetMesh())
	{
		GetMesh()->SetAnimInstanceClass(NewAnimBP);

		if (UPlayerCharacterAnimInstance* Anim = Cast<UPlayerCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
		{
			Anim->OnAttackStart.RemoveAll(this);
			Anim->OnAttackStart.AddUObject(this, &APlayerCharacter::HandleAttackStartNotify);

			Anim->OnSFX_Player_Footstep1.RemoveAll(this);
			Anim->OnSFX_Player_Footstep1.AddUObject(this, &APlayerCharacter::HandleFootstepLeft);

			Anim->OnSFX_Player_Footstep2.RemoveAll(this);
			Anim->OnSFX_Player_Footstep2.AddUObject(this, &APlayerCharacter::HandleFootstepRight);

		}

	}

	if (Axe)
	{
		Axe->SetActorHiddenInGame(!bShowWeapon);
	}

	UE_LOG(LogTemp, Warning, TEXT("외형 변경 완료: %d"), (int32)NewRole);
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

		if (HasAuthority())
		{
			SetActorRotation(NewRot);
		}

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

	if (!Controller) return;

	bAttackStartFired = false;

	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		if (AttackMontage)
		{
			Anim->Montage_Play(AttackMontage);
		}
	}

	const FRotator ControlRot = Controller->GetControlRotation();
	AttackLockedYaw = ControlRot.Yaw;
	bIsAttacking = true;

	GetWorldTimerManager().ClearTimer(AttackFaceOffTimerHandle);
	GetWorldTimerManager().SetTimer(
		AttackFaceOffTimerHandle,
		[this]() { bIsAttacking = false; },
		AttackFaceHoldTime,
		false
	);

	const float Yaw = Controller
		? Controller->GetControlRotation().Yaw
		: GetActorRotation().Yaw;

	Server_StartAttackSync(Yaw);
}

bool APlayerCharacter::Server_Attack_Validate(FVector Start, FVector Dir)
{
	return true;
}

void APlayerCharacter::Server_Attack_Implementation(FVector Start, FVector Dir)
{

	FHitResult Hit;
	const bool bHit = DoLineTraceFromAxe(Hit);

	if (AT5GameMode* GM = Cast<AT5GameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ProcessAttack(GetController(), bHit ? Hit.GetActor() : nullptr);
	}

	if (IsHunter())
	{
		if (bHit)
		{
			Multicast_PlaySound(AttackHitSound);
		}
		else
		{
			Multicast_PlaySound(AttackMissSound);
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
	SetLifeSpan(5.0f);
}

bool APlayerCharacter::DoLineTraceFromAxe(FHitResult& OutHit) const
{
	if (!Axe) return false;

	UE_LOG(LogTemp, Warning, TEXT("AxeTrace called on %s | HasAuthority=%d | LocallyControlled=%d | Axe=%s"),
		HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
		HasAuthority(),
		IsLocallyControlled(),
		Axe ? *Axe->GetName() : TEXT("NULL"));

	UStaticMeshComponent* Axe_Mesh = Axe->GetAxeMesh();
	if (!Axe_Mesh) return false;

	FVector Start;
	FRotator Rot;

	if (Axe_Mesh->DoesSocketExist(TEXT("TraceStart")))
	{
		Start = Axe_Mesh->GetSocketLocation(TEXT("TraceStart"));
		Rot = Axe_Mesh->GetSocketRotation(TEXT("TraceStart"));
	}
	else
	{
		Start = Axe_Mesh->GetComponentLocation();
		Rot = Axe_Mesh->GetComponentRotation();
	}

	FRotator YawRot(0.f, AttackLockedYaw, 0.f);
	FVector Dir = YawRot.Vector();

	const FVector End = Start + Dir * TraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AxeTrace), true);
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Axe);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit, Start, End, ECC_Visibility, Params
	);

	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		bHit ? FColor::Red : FColor::Green,
		false,
		1.0f,
		0,
		2.0f
	);

	return bHit;
}

void APlayerCharacter::HandleAttackStartNotify()
{
	if (!IsLocallyControlled()) return;
	if (!Controller) return;

	/*FHitResult Dummy;
	DoLineTraceFromAxe(Dummy);*/

	FVector Start; FRotator Rot;
	Controller->GetPlayerViewPoint(Start, Rot);
	Server_Attack(Start, Rot.Vector());
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, Axe);
	DOREPLIFETIME(APlayerCharacter, bIsAttacking);
	DOREPLIFETIME(APlayerCharacter, AttackLockedYaw);
}

void APlayerCharacter::OnRep_Axe()
{
	if (!Axe) return;

	// 클라에서 복제된 Axe를 내 Mesh에 부착
	Axe->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("RightHandSocket"));

	// 컨트롤러 참조 세팅 (로컬 컨트롤러가 있을 때만)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		Axe->OwnerController = PC;
	}
}

bool APlayerCharacter::Server_StartAttackSync_Validate(float InLockedYaw)
{
	return true;
}

void APlayerCharacter::Server_StartAttackSync_Implementation(float InLockedYaw)
{

	UE_LOG(LogTemp, Warning, TEXT("[Server_StartAttackSync] Auth=%d Pawn=%s"), HasAuthority(), *GetName());

	AttackLockedYaw = InLockedYaw;
	bIsAttacking = true;

	Multicast_PlayAttackMontage();

	GetWorldTimerManager().ClearTimer(AttackFaceOffTimerHandle);
	GetWorldTimerManager().SetTimer(
		AttackFaceOffTimerHandle,
		[this]() { bIsAttacking = false; },
		AttackFaceHoldTime,
		false
	);
}

void APlayerCharacter::Multicast_PlayAttackMontage_Implementation()
{

	UE_LOG(LogTemp, Warning, TEXT("[Multicast Fired] Local=%d Auth=%d Pawn=%s Montage=%s Anim=%s"),
		IsLocallyControlled(),
		HasAuthority(),
		*GetName(),
		*GetNameSafe(AttackMontage),
		*GetNameSafe(GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	);

	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		if (AttackMontage)
		{
			// 중복 재생 방지(필요하면)
			if (!Anim->Montage_IsPlaying(AttackMontage))
			{
				const float Len = Anim->Montage_Play(AttackMontage);
				UE_LOG(LogTemp, Warning, TEXT("[Multicast Montage_Play] Len=%f Pawn=%s"), Len, *GetName());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Multicast] AnimInstance NULL Pawn=%s"), *GetName());
	}
}

bool APlayerCharacter::IsHunter() const
{
	if (const AT5PlayerState* PS = GetPlayerState<AT5PlayerState>())
	{
		return PS->CurrentRole == EPlayerRole::Hunter;
	}
	return false;
}

void APlayerCharacter::Multicast_PlaySound_Implementation(USoundBase* Sound)
{
	if (!Sound) return;

	UGameplayStatics::PlaySoundAtLocation(
		this,
		Sound,
		GetActorLocation()
	);
}

void APlayerCharacter::HandleFootstepLeft()
{
	if (!HasAuthority()) return;
	if (!IsHunter()) return;

	Multicast_PlaySound(FootstepLeftSound);
}

void APlayerCharacter::HandleFootstepRight()
{
	if (!HasAuthority()) return;
	if (!IsHunter()) return;

	Multicast_PlaySound(FootstepRightSound);
}

void APlayerCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	if (HasAuthority() && IsHunter())
	{
		Multicast_PlaySound(JumpStartSound);
	}
}


void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (HasAuthority() && IsHunter())
	{
		Multicast_PlaySound(JumpLandSound);
	}
}
