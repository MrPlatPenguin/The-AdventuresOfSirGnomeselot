// Fill out your copyright notice in the Description page of Project Settings.


#include "GardenGameCharacter.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include <iostream>
#include "EnhancedInputComponent.h"

// Sets default values
AGardenGameCharacter::AGardenGameCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGardenGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	Initialize();
}

// Called to bind functionality to input
void AGardenGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);
		}
	}
	if (UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &AGardenGameCharacter::JumpPressed);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &AGardenGameCharacter::JumpReleased);
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGardenGameCharacter::MoveInput);
		Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &AGardenGameCharacter::ClearMoveInput);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGardenGameCharacter::CameraLook);
		Input->BindAction(DodgeAction, ETriggerEvent::Started, this, &AGardenGameCharacter::DodgePressed);
		Input->BindAction(DodgeAction, ETriggerEvent::Completed, this, &AGardenGameCharacter::DodgeReleased);
		Input->BindAction(AttackAction, ETriggerEvent::Started, this, &AGardenGameCharacter::AttackPressed);
		Input->BindAction(AttackAction, ETriggerEvent::Completed, this, &AGardenGameCharacter::AttackReleased);
		//Input->BindAction(ThrowSeedAction, ETriggerEvent::Started, this, &AGardenGameCharacter::ThrowSeedPressed);
		//Input->BindAction(ThrowSeedAction, ETriggerEvent::Completed, this, &AGardenGameCharacter::ThrowSeedRelease);
	}
}

// Called every frame
void AGardenGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DeltaT = DeltaTime;
	UpdateChachedVelocity();

	switch (CurrentState)
	{
	case CharacterState::Idle:
		IdleTick();
		break;
	case CharacterState::Grounded:
		GroundedTick();
		CheckGroundedExitConidtions();
		break;
	case CharacterState::Jumping:
		JumpingTick();
		CheckJumpExitConidtions();
		break;
	case CharacterState::Falling:
		FallingTick();
		break;
	case CharacterState::Gliding:
		GlidingTick();
		break;
	case CharacterState::GlidingBoosted:
		GlidingBoostTick();
		break;
	case CharacterState::Dodging:
		DodgeTick();
		break;
	case CharacterState::Attacking:
		AttackTick();
		break;
	case CharacterState::Stunned:
		StunTick();
		break;
	case CharacterState::ThrowingSeed:
		ThrowingSeedTick();
		break;
	case CharacterState::Cheering:
		CheeringTick();
		break;
	case CharacterState::Sliding:
		SlidingTick();
		break;
	case CharacterState::NoMovement:
		NoMovementTick();
		break;
	case CharacterState::NoInput:
		NoInputTick();
		break;
	default:
		break;
	}
	RelativeTeleport();
	TeleportToLocation();
	UpdateComponentVelocity();
}

void AGardenGameCharacter::Initialize()
{
	Collider = FindComponentByClass<UCapsuleComponent>();
	CharacterHalfHeight = Collider->GetScaledCapsuleHalfHeight();
	GroundCheckRadius = Collider->GetUnscaledCapsuleRadius();

	MovementComponent = FindComponentByClass<UFloatingPawnMovement>();

	SpringArm = FindComponentByClass<USpringArmComponent>();
	SpringArm->TargetArmLength = playerData->CameraDistance;
	MaxHealth = playerData->StartingHealth + BonusHealth;

	CurrentDodgeState = NotDodging;

	GroundedEnter();
	RestoreMaxHeatlh();

	StaticCamera = GetWorld()->SpawnActor<AStaticCamera>();
}

void AGardenGameCharacter::UpdateChachedVelocity()
{
	Velocity = MovementComponent->Velocity;

}

void AGardenGameCharacter::UpdateComponentVelocity()
{
	MovementComponent->Velocity = Velocity;
	if (ExternalVelocity.Length() > 0)
	{
		MovementComponent->Velocity = ExternalVelocity;
		ExternalVelocity = FVector::ZeroVector;
	}
}

bool AGardenGameCharacter::GetGround(FHitResult& HitResult)
{
	if (!GetWorld())
		return false;
	FVector ActorLocation = GetActorLocation() + (FVector::DownVector * (CharacterHalfHeight - GroundCheckRadius));
	FVector Start = ActorLocation;
	FVector End = ActorLocation - FVector(0.0f, 0.0f, playerData->GroundingDistance);

	FCollisionQueryParams TraceParams(FName(TEXT("GroundTrace")), false, this);
	TraceParams.bIgnoreTouches = true;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(GroundCheckRadius);
	bool bHit = false;
	// Keeps searching for ground until a non-trigger collider is found
	bool TransparentColliderFound = true;
	while (TransparentColliderFound)
	{
		bHit = GetWorld()->SweepSingleByObjectType(
			HitResult,
			Start,
			End,
			FQuat::Identity,
			FCollisionObjectQueryParams::AllObjects,
			Sphere,
			TraceParams
		);
		TransparentColliderFound = bHit && HitResult.GetComponent()->GetCollisionEnabled() == ECollisionEnabled::QueryOnly;
		TraceParams.AddIgnoredComponent(HitResult.GetComponent());
	}

	//DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, GroundCheckRadius, 26, FColor::Red);
	return bHit && HitResult.GetActor() != this;
}

bool AGardenGameCharacter::GetGround()
{
	FHitResult HitResult;
	return GetGround(HitResult);
}

bool AGardenGameCharacter::GetGroundValidAngle(FHitResult& HitResult)
{
	return GetGround(HitResult) && ValidGroundAngle(HitResult);
}

bool AGardenGameCharacter::GetGroundValidAngle()
{
	FHitResult HitResult;
	return GetGroundValidAngle(HitResult);
}

void AGardenGameCharacter::HandleGravity(float Acceleration, float MaxFallSpeed)
{
	float newFallSpeed = Velocity.Z - (Acceleration * DeltaT);
	newFallSpeed = FMath::Clamp(newFallSpeed, -MaxFallSpeed, FLT_MAX);
	Velocity.Z = newFallSpeed;
}

void AGardenGameCharacter::GroundedCheck()
{
	FHitResult HitResult;
	if (!GetGround(HitResult))
		return;

	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, HitResult.GetActor()->GetName());
	if (ValidGroundAngle(HitResult))
		GroundedEnter();
	else
		SlidingEnter();
}

void AGardenGameCharacter::StickToGround()
{
	FHitResult HitResult;
	if (!GetGroundValidAngle(HitResult))
	{
		FallingEnter();
		return;
	}

	FVector GroundPoint = HitResult.ImpactPoint;

	if (HitResult.Distance > 0)
		SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GroundPoint.Z) + FVector::UpVector * CharacterHalfHeight);

	return;
}

void AGardenGameCharacter::OnGrounded()
{
	DodgeConsumed = false;
	IsGlideHeld = false;
	CoyotteAvailable = true;
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Grounded");
}

void AGardenGameCharacter::HandleMove(float AccelerationSpeed, float DecelerationSpeed, float MaxSpeed)
{
	FVector CurrentHorizonalVelocity = FVector(Velocity.X, Velocity.Y, 0);
	FVector TargetVelocity = moveVector * MaxSpeed;
	float CurrentSpeed = CurrentHorizonalVelocity.Length();
	float TargetSpeed = TargetVelocity.Length();

	FVector NewHorizontalVelocity;
	if (TargetSpeed == 0 || AccelerationSpeed == 0) //Decelerating
		NewHorizontalVelocity = MoveVectorTowards(CurrentHorizonalVelocity, FVector::ZeroVector, DecelerationSpeed * DeltaT);
	else //Accelerating
		NewHorizontalVelocity = MoveVectorTowards(CurrentHorizonalVelocity, TargetVelocity, AccelerationSpeed * DeltaT);

	Velocity.X = NewHorizontalVelocity.X;
	Velocity.Y = NewHorizontalVelocity.Y;
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Move");
}

void AGardenGameCharacter::HandleGroundedMove(float AccelerationSpeed, float DecelerationSpeed, float MaxSpeed)
{
	FHitResult GroundImpact;
	if (!GetGroundValidAngle(GroundImpact))
	{
		FallingEnter();
		return;
	}

	FVector GroundNormal = GroundImpact.ImpactNormal;

	FVector RotatatedMoveVector = FVector::VectorPlaneProject(moveVector, GroundNormal);

	FVector CurrentHorizonalVelocity = Velocity;
	FVector TargetVelocity = RotatatedMoveVector * MaxSpeed;
	float CurrentSpeed = CurrentHorizonalVelocity.Length();
	float TargetSpeed = TargetVelocity.Length();

	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (CurrentHorizonalVelocity *500.f), FColor::Red, false, 0.02f);
	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (RotatatedMoveVector *500.f), FColor::Red, false, 0.02f);

	FVector NewHorizontalVelocity;
	if (TargetSpeed == 0 || AccelerationSpeed == 0) //Decelerating
		NewHorizontalVelocity = MoveVectorTowards(CurrentHorizonalVelocity, FVector::ZeroVector, DecelerationSpeed * DeltaT);
	else //Accelerating
		NewHorizontalVelocity = MoveVectorTowards(CurrentHorizonalVelocity, TargetVelocity, AccelerationSpeed * DeltaT);

	Velocity = NewHorizontalVelocity;

	// Stick player to ground
	FVector GroundPoint = GroundImpact.ImpactPoint;

	if (GroundImpact.Distance > 0)
		SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GroundPoint.Z) + FVector::UpVector * CharacterHalfHeight);

	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Move");
}

void AGardenGameCharacter::PointCharacterForwards()
{
	if (moveVector.Size() > 0)
		SetActorRotation(FVector(Velocity.X, Velocity.Y, 0).Rotation());
}

void AGardenGameCharacter::PointCharacterTowardCamera()
{
	SetActorRotation(GetFlatControlRotation());
}

FVector AGardenGameCharacter::GetForwardVector()
{
	if (InCombat)
	{
		FRotator rotation = CombatCameraDirection.Rotation();
		rotation.Pitch = 0.f;
		rotation.Roll = 0.f;
		return UKismetMathLibrary::GetForwardVector(rotation);
	}
	else
		return UKismetMathLibrary::GetForwardVector(GetFlatControlRotation());
}

FVector AGardenGameCharacter::GetRightVector()
{
	if (InCombat)
	{
		FRotator rotation = CombatCameraDirection.Rotation();
		rotation.Pitch = 0.f;
		rotation.Roll = 0.f;
		return UKismetMathLibrary::GetRightVector(rotation);
	}
	else
		return UKismetMathLibrary::GetRightVector(GetFlatControlRotation());
}

TArray<AEnemyTurret*> AGardenGameCharacter::CheckForEnemies()
{
	UWorld* World = GetWorld();
	TArray<AEnemyTurret*> OverlappingActors;
	// Ensure the world context is valid
	if (!World) return OverlappingActors;

	DrawDebugSphere(GetWorld(), GetActorLocation(), playerData->AttackRange, 16, FColor::Red, false, 0.02f);

	FVector SphereCenter = GetActorLocation();
	float SphereRadius = playerData->AttackRange;

	// Sphere overlap parameters
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SphereRadius);
	TArray<FHitResult> HitResults;

	// Perform the sphere overlap
	bool bOverlapResult = World->SweepMultiByObjectType(
		HitResults,
		SphereCenter,
		SphereCenter,
		FQuat::Identity,
		FCollisionObjectQueryParams::AllObjects,
		Sphere
	);

	// If the overlap found any actors
	if (bOverlapResult)
	{
		for (FHitResult HitResult : HitResults)
		{
			AEnemyTurret* Enemy = Cast<AEnemyTurret>(HitResult.GetActor());
			if (Enemy)
				OverlappingActors.Add(Enemy);
		}
	}
	return OverlappingActors;
}

FVector AGardenGameCharacter::MoveVectorTowards(FVector current, FVector target, float maxDistanceDelta)
{
	FVector a = target - current;
	float magnitude = a.Length();
	if (magnitude <= maxDistanceDelta || magnitude == 0.f)
	{
		return target;
	}
	return current + a / magnitude * maxDistanceDelta;
	return FVector();
}

FRotator AGardenGameCharacter::GetFlatControlRotation()
{
	FRotator ControlRotation = GetControlRotation().GetNormalized();
	ControlRotation = FRotator(0.f, ControlRotation.Yaw, 0.f);
	return ControlRotation;
}

void AGardenGameCharacter::TakeDamage(int damage)
{
	if (CurrentDodgeState == DodgeState::NotDodging && !(CurrentState == CharacterState::Stunned)) {
		Health -= damage;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Player Damaged"));

		OnHealthChange.Broadcast();

		StunEnter();

		if (Health <= 0)
			Die();
	}
	else if (CurrentDodgeState == DodgeState::PerfectDodge)
		PerfectDodgePerformed();
}

void AGardenGameCharacter::RestoreMaxHeatlh()
{
	Health = MaxHealth + BonusHealth;
	OnHealthChange.Broadcast();
}

void AGardenGameCharacter::SetBonusHealth(int Value)
{
	BonusHealth = Value;
	OnHealthChange.Broadcast();
}

void AGardenGameCharacter::Die()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Player Died"));
}

void AGardenGameCharacter::AddRelativeTeleport(FVector Distance)
{
	RelativeTeleportVector += Distance;
}

void AGardenGameCharacter::Teleport(FVector Location)
{
	TeleportLocation = Location;
}

void AGardenGameCharacter::TeleportToLocation()
{
	if (TeleportLocation.Length() == 0)
		return;

	SetActorLocation(TeleportLocation, false, nullptr, ETeleportType::TeleportPhysics);
	Velocity = FVector::ZeroVector;
	MovementComponent->Velocity = Velocity;
	TeleportLocation = FVector::ZeroVector;
}

void AGardenGameCharacter::RelativeTeleport()
{
	//RelativeTeleportVector.Z += 1.f;
	SetActorLocation(GetActorLocation() + RelativeTeleportVector, false);

	RelativeTeleportVector = FVector::ZeroVector;
}

void AGardenGameCharacter::SetVeloctiy(FVector NewVelocity)
{
	ExternalVelocity = NewVelocity;
}

FVector AGardenGameCharacter::GetVeloctiy()
{
	return Velocity;
}

void AGardenGameCharacter::HandleWallBounce()
{
	TimeSinceLastWallBounce += DeltaT;
	if (TimeSinceLastWallBounce < 0.2f)
		return;
	UWorld* World = GetWorld();
	// Ensure the world context is valid
	if (!World) return;

	FVector SphereCenter = GetActorLocation();
	float SphereRadius = playerData->WallBounceCheckDistance;

	// Sphere overlap parameters
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SphereRadius);
	TArray<FHitResult> HitResults;

	// Perform the sphere overlap
	bool bOverlapResult = World->SweepMultiByObjectType(
		HitResults,
		SphereCenter,
		SphereCenter,
		FQuat::Identity,
		FCollisionObjectQueryParams::AllStaticObjects,
		Sphere
	);

	if (!bOverlapResult)
		return;

	for (FHitResult HitResult : HitResults)
	{
		FVector ImpactDirection = (HitResult.ImpactPoint - GetActorLocation()).GetSafeNormal();
		float ImpactDownAngleDeg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ImpactDirection, FVector::DownVector)));
		float ImpactUpAngleDeg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ImpactDirection, FVector::UpVector)));
		bool ValidAngle = ImpactDownAngleDeg > playerData->WallBounceAngle && ImpactUpAngleDeg > playerData->WallBounceAngle;
		if (ValidAngle)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, HitResult.GetActor()->GetName());
			Velocity += HitResult.ImpactNormal * playerData->WallBounceForce;// * GetAttackSpinUpAlpha();
			TimeSinceLastWallBounce = 0;
			AttackSpinTime *= playerData->WallBounceSpeedReductionFactor;
			break;
		}
	}
}

FVector AGardenGameCharacter::GetThrowLandingPoint()
{
	FVector ForwardVector = GetActorForwardVector();
	FVector CameraForwardVector = UKismetMathLibrary::GetForwardVector(GetControlRotation());
	float Dot = FVector::DotProduct(ForwardVector, CameraForwardVector);
	FVector Offset = GetActorForwardVector() * playerData->PlantingThrowRange * Dot;
	FVector LandPoint = GetActorLocation() + Offset;
	FHitResult HitResult;

	return GetActorLocation() + Offset;
}

void AGardenGameCharacter::RemoveInputForPlayer(bool doPhysics)
{
	if (doPhysics)
		CurrentState = CharacterState::NoInput;
	else
		CurrentState = CharacterState::NoMovement;
}

void AGardenGameCharacter::ReturnInputForPlayer()
{
	CurrentState = CharacterState::Falling;
}

void AGardenGameCharacter::StartCheering(AActor* DisplayActor)
{
	CurrentState = CharacterState::Cheering;
	CheeringItem = GetWorld()->SpawnActor<AActor>();
	CheeringItem->SetActorLocation(GetActorLocation() + (FVector::UpVector * 100.f));
	CheeringTimeRemaining = playerData->CheeringDuration;
}

void AGardenGameCharacter::StopCheering()
{
	GroundedEnter();
	CheeringItem->Destroy();
}

void AGardenGameCharacter::SetPlayerStaticCameraLocation(FVector Location, FVector ForwardDirection, float Speed)
{
	StaticCamera->SetCameraLocation(Location, ForwardDirection, Speed);
}

void AGardenGameCharacter::ReturnPlayerCameraLocation(float Speed)
{
	UGameplayStatics::GetPlayerController(this, 0)->SetViewTargetWithBlend(this, Speed);
}

void AGardenGameCharacter::CharacterLookAt(FVector point)
{
	point.Z = GetActorLocation().Z;
	FVector Direction = (point - GetActorLocation()).GetSafeNormal();
	SetActorRotation(Direction.Rotation());
}

void AGardenGameCharacter::PerfectDodgePerformed()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), playerData->PerfectDodgeSlowMotionFactor);
	DidPerfectDodge = true;
}

bool AGardenGameCharacter::ValidGroundAngle(FHitResult HitResult)
{
	float GroundAngle = ((acosf(FVector::DotProduct(HitResult.ImpactNormal, FVector::UpVector))) * (180 / 3.1415926));
	bool ValidGroundAngle = GroundAngle <= playerData->MaxGroundSlopeAngle;
	return ValidGroundAngle;
}

float AGardenGameCharacter::GetAttackSpinUpAlpha()
{
	float SpinAlpha = playerData->AttackSpinUpCurve.GetRichCurveConst()->Eval(AttackSpinTime / playerData->SpinUpTime);
	FMath::Clamp(SpinAlpha, 0, 1);
	return SpinAlpha;
}

float AGardenGameCharacter::GetSpinSpeed()
{
	return FMath::Lerp(0, playerData->MaxRotationSpeed, GetAttackSpinUpAlpha());
}

void AGardenGameCharacter::MoveInput(const FInputActionValue& Value)
{

	FVector LocalMovementVector = (GetForwardVector() * Value.Get<FVector2D>().Y) + (GetRightVector() * Value.Get<FVector2D>().X);

	moveVector = LocalMovementVector;
	moveVector.Normalize();
}

void AGardenGameCharacter::CameraLook(const FInputActionValue& Value)
{
	AddControllerYawInput(Value.Get<FVector2D>().X * playerData->CameraHorizontalSensitivity * DeltaT);
	AddControllerPitchInput(Value.Get<FVector2D>().Y * playerData->CameraVerticalSensitivity * DeltaT);
}

void AGardenGameCharacter::JumpPressed()
{
	IsJumpPressed = true;
	IsGlideHeld = CurrentState == CharacterState::Falling || CurrentState == CharacterState::Jumping;
}

void AGardenGameCharacter::JumpReleased()
{
	IsJumpPressed = false;
	IsGlideHeld = false;
}

void AGardenGameCharacter::DodgePressed()
{
	IsDodgePressed = true;
}

void AGardenGameCharacter::DodgeReleased()
{
	IsDodgePressed = false;
}

void AGardenGameCharacter::AttackPressed()
{
	IsAttackPressed = true;
}

void AGardenGameCharacter::AttackReleased()
{
	IsAttackPressed = false;
}

void AGardenGameCharacter::ClearMoveInput()
{
	moveVector = FVector(0, 0, 0);
}

void AGardenGameCharacter::ThrowSeedPressed()
{
	IsThrowSeedPressed = true;
}

void AGardenGameCharacter::ThrowSeedRelease()
{
	IsThrowSeedPressed = false;
}

void AGardenGameCharacter::IdleEnter()
{
	bUseControllerRotationYaw = false;
}

void AGardenGameCharacter::IdleTick()
{
}

void AGardenGameCharacter::GroundedEnter()
{
	CurrentState = CharacterState::Grounded;
	OnGrounded();
}

void AGardenGameCharacter::GroundedTick()
{
	HandleGroundedMove(playerData->BaseMoveAcceleration, playerData->BaseMoveDeceleration, playerData->BaseMoveSpeed);
	PointCharacterForwards();
	StickToGround();
}

void AGardenGameCharacter::CheckGroundedExitConidtions()
{
	CheckJumpEnter();
	CheckDodgeEnter();
	CheckAttackEnter();
	CheckThrowingSeedEnter();
}

void AGardenGameCharacter::EnterJump()
{
	CurrentState = CharacterState::Jumping;
	JumpHeldTime = 0;
	CoyotteAvailable = false;
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Jumped");
}

bool AGardenGameCharacter::CheckJumpEnter()
{
	if (IsJumpPressed)
	{
		EnterJump();
		return true;
	}
	return false;
}

void AGardenGameCharacter::JumpingTick()
{
	HandleMove(playerData->FallHorizontalAcceleration, playerData->FallHorizontalDeceleration, playerData->BaseMoveSpeed);
	PointCharacterForwards();
	JumpHeldTime += DeltaT;
	Velocity.Z = playerData->JumpForce;
	CheckDodgeEnter();
}

void AGardenGameCharacter::CheckJumpExitConidtions()
{
	if (!((IsJumpPressed && JumpHeldTime <= playerData->MaxJumpHoldTime) || JumpHeldTime < playerData->MinJumpHoldTime))
		FallingEnter();
}

void AGardenGameCharacter::FallingEnter()
{
	CurrentState = CharacterState::Falling;
	IsJumpPressed = false;
	TimeStartedFalling = 0.f;
}

void AGardenGameCharacter::FallingTick()
{
	HandleMove(playerData->FallHorizontalAcceleration, playerData->FallHorizontalDeceleration, playerData->BaseMoveSpeed);
	PointCharacterForwards();
	HandleGravity(playerData->FallAcceleration, playerData->MaxFallSpeed);
	GroundedCheck();

	// Exit
	TimeStartedFalling += DeltaT;
	if (TimeStartedFalling < playerData->CoyotteTime && CoyotteAvailable)
		CheckJumpEnter();
	else
	{
		CheckGlideEnter();
		CheckDodgeEnter();
	}
}

void AGardenGameCharacter::CheckDodgeEnter()
{
	if (IsDodgePressed && !DodgeConsumed)
		DodgeEnter();
}

void AGardenGameCharacter::DodgeEnter()
{
	DodgeStartPos = GetActorLocation();
	FVector DodgeDirection = moveVector.Length() > 0 ? moveVector : GetActorForwardVector();
	DodgeEndPos = DodgeStartPos + (DodgeDirection * playerData->DodgeDistance);
	DodgeEndPos.Z += 0.1f;
	CurrentState = CharacterState::Dodging;
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Dodge");
	DodgeTime = 0.f;
	Velocity = FVector::ZeroVector;
	DodgeConsumed = true;
}

void AGardenGameCharacter::DodgeTick()
{
	DodgeTime += DeltaT;

	float DodgeAlpha = playerData->DodgeSpeedCurve.GetRichCurveConst()->Eval(DodgeTime / playerData->DodgeSpeed);
	FMath::Clamp(DodgeAlpha, 0, 1);
	FVector NewLocation = FMath::Lerp(DodgeStartPos, DodgeEndPos, DodgeAlpha);

	if (DodgeAlpha <= playerData->PerfectDodgeWindow) {
		CurrentDodgeState = PerfectDodge;
	}
	else
		CurrentDodgeState = StandardDodge;


	//SetActorLocation(NewLocation, true);

	Velocity = (NewLocation - GetActorLocation()) / DeltaT;
	

	// Exit
	if (DodgeAlpha < 1)
		return;
	CurrentDodgeState = NotDodging;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);

	if (!DidPerfectDodge)
		AttackSpinTime *= playerData->DodgeSlowSpinFactor;

	if (GetGroundValidAngle())
		GroundedEnter();
	else
		FallingEnter();
}

void AGardenGameCharacter::CheckGlideEnter()
{
	if (IsGlideHeld && GlideUnlocked)
		GlideEnter();
}

void AGardenGameCharacter::GlideEnter()
{
	CurrentState = CharacterState::Gliding;
}

void AGardenGameCharacter::GlidingTick()
{
	HandleMove(playerData->GlideHorizontalAcceleration, playerData->GlideHorizontalDeceleration, playerData->GlideMoveSpeed);
	HandleGravity(playerData->FallAcceleration, playerData->MaxGlideFallSpeed);
	PointCharacterForwards();

	// Exit
	if (!IsJumpPressed)
		FallingEnter();
	CheckGlideBoostEnter();
	GroundedCheck();
}

void AGardenGameCharacter::CheckGlideBoostEnter()
{
	if (GlideBoostDirection.Length() > 0)
		GlideBoostEnter();
}

void AGardenGameCharacter::GlideBoostEnter()
{
	CurrentState = CharacterState::GlidingBoosted;
}

void AGardenGameCharacter::GlidingBoostTick()
{
	HandleMove(playerData->GlideHorizontalAcceleration, playerData->GlideHorizontalDeceleration, playerData->GlideMoveSpeed);
	PointCharacterForwards();
	Velocity += GlideBoostDirection * playerData->BoostAcceleration * DeltaT;
	Velocity = Velocity.GetClampedToMaxSize(playerData->MaxGlideBoostSpeed);

	// Exit
	if (GlideBoostDirection.Length() == 0)
		GlideEnter();
	if (!IsJumpPressed)
		FallingEnter();
}

void AGardenGameCharacter::CheckAttackEnter()
{
	if (IsAttackPressed)
		AttackEnter();
}

void AGardenGameCharacter::AttackEnter()
{
	CurrentState = CharacterState::Attacking;
	DodgeConsumed = false;
}

void AGardenGameCharacter::AttackTick()
{
	AttackSpinTime = FMath::Clamp(AttackSpinTime + DeltaT,0, playerData->SpinUpTime);

	HandleGroundedMove(playerData->AttackingMoveAcceleration, playerData->AttackingMoveDeceleration, playerData->AttackingMoveSpeed);
	HandleGravity(playerData->FallAcceleration, playerData->MaxFallSpeed);

	// Try Attack
	if (GetAttackSpinUpAlpha() >= 1)
	{
		TArray<AEnemyTurret*> Enemeis = CheckForEnemies();
		if (Enemeis.Num() > 0)
		{
			for (AEnemyTurret* Enemy : Enemeis)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, Enemy->GetName());
				Enemy->KillEnemy();
			}
		}
	}

	// Wall Bounce
	HandleWallBounce();

	// Exit
	CheckDodgeEnter();

	if (IsAttackPressed)
		return;
	AttackSpinTime = 0.f;

	if (GetGroundValidAngle())
		GroundedEnter();
	else
		FallingEnter();
}

void AGardenGameCharacter::StunEnter()
{
	CurrentState = CharacterState::Stunned;
	StunTimer = 0;
}

void AGardenGameCharacter::StunTick()
{
	StunTimer += DeltaT;

	HandleGravity(playerData->FallAcceleration, playerData->MaxFallSpeed);
	HandleGroundedMove(0, playerData->BaseMoveDeceleration, playerData->AttackingMoveSpeed);

	// Exit
	if (StunTimer >= playerData->StunTime)
		GroundedEnter();
}

void AGardenGameCharacter::CheckThrowingSeedEnter()
{
	if (IsThrowSeedPressed)
		ThrowingSeedEnter();
}

void AGardenGameCharacter::ThrowingSeedEnter()
{
	CurrentState = CharacterState::ThrowingSeed;
	FVector SpawnPoint = GetThrowLandingPoint();
	ThrowVisualSpawnActorInstance = GetWorld()->SpawnActor<AActor>(ThrowVisualSpawnActor, SpawnPoint, GetActorRotation());
}

void AGardenGameCharacter::ThrowingSeedTick()
{
	PointCharacterTowardCamera();

	ThrowVisualSpawnActorInstance->SetActorLocation(GetThrowLandingPoint());

	// Exit
	if (IsThrowSeedPressed)
		return;

	ThrowVisualSpawnActorInstance->Destroy();

	GroundedEnter();
}

void AGardenGameCharacter::CheeringTick()
{
	HandleMove(0, 99999.f, 0.f);

	CheeringTimeRemaining -= DeltaT;

	if (CheeringTimeRemaining <= 0)
		CheeringExit();
}

void AGardenGameCharacter::CheeringExit()
{
	ReturnPlayerCameraLocation(1.f);
	GroundedEnter();
}

void AGardenGameCharacter::SlidingEnter()
{
	CurrentState = CharacterState::Sliding;
}

void AGardenGameCharacter::SlidingTick()
{
	HandleMove(playerData->FallHorizontalAcceleration, playerData->FallHorizontalDeceleration, playerData->BaseMoveSpeed);
	PointCharacterForwards();
	HandleGravity(playerData->FallAcceleration, playerData->MaxFallSpeed);

	FHitResult HitResult;
	if (!GetGround(HitResult))
	{
		FallingEnter();
	}

	if (ValidGroundAngle(HitResult))
		GroundedEnter();
}

void AGardenGameCharacter::NoMovementTick()
{
	Velocity = FVector::ZeroVector;
}

void AGardenGameCharacter::NoInputTick()
{
	HandleMove(0, 99999.f, 0.f);
}