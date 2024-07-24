// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerStatsDataAsset.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"
#include <Kismet/GameplayStatics.h>
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/ArrowComponent.h"
#include "EnemyTurret.h"
#include "StaticCamera.h"
#include "GardenGameCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerEvent);

UENUM(BlueprintType)
enum class CharacterState : uint8
{
	Idle,
	Grounded,
	Jumping,
	Falling,
	Gliding,
	GlidingBoosted,
	Dodging,
	Attacking,
	Stunned,
	ThrowingSeed,
	Cheering,
	Sliding,
	NoInput,
	NoMovement
};

UENUM(BlueprintType)
enum DodgeState : uint8
{
	NotDodging,
	PerfectDodge,
	StandardDodge
};

UCLASS()
class GARDENGAME_API AGardenGameCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGardenGameCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// Components
	UCapsuleComponent* Collider;
	UFloatingPawnMovement* MovementComponent;
	USpringArmComponent* SpringArm;
	UPROPERTY(EditDefaultsOnly)
		UActorComponent* MeshComp;
	AStaticCamera* StaticCamera;

	// Events

	// General
	float GroundCheckRadius;
	float DeltaT;
	FVector moveVector;
	float CharacterHalfHeight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		CharacterState CurrentState;
	FVector Velocity;
	FVector RelativeTeleportVector;
	FVector TeleportLocation;
	FVector ExternalVelocity;

	// Jumping
	bool IsJumpPressed;
	float JumpHeldTime;
	float TimeStartedFalling;
	bool CoyotteAvailable;

	// Gliding
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector GlideBoostDirection;
	bool JumpReleasedBeforeHold;
	bool IsGlideHeld;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool GlideUnlocked;

	// Dodge
	bool IsDodgePressed;
	FVector DodgeStartPos;
	FVector DodgeEndPos;
	float DodgeTime;
	bool DodgeConsumed;
	DodgeState CurrentDodgeState;
	bool DidPerfectDodge;

	// Combat
	bool IsAttackPressed;
	float AttackSpinTime;
	UPROPERTY(BlueprintReadOnly)
		float Health;
	float BonusHealth;
	UPROPERTY(BlueprintReadOnly)
		int MaxHealth;
	float TimeSinceTakenDamage;
	UPROPERTY(BlueprintReadWrite)
		float SwordRange;
	UPROPERTY(BlueprintReadWrite)
		float SwordSpinUpSpeed;
	UPROPERTY(BlueprintReadWrite)
		bool InCombat;
	UPROPERTY(BlueprintReadWrite)
		FVector CombatCameraDirection;
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FPlayerEvent OnHealthChange;
	float TimeSinceLastWallBounce;

	// Stun
	float StunTimer;

	// Planting
	bool IsThrowSeedPressed;
	int CurrentThrowAmmoIndex;
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AActor>ThrowVisualSpawnActor;
	AActor* ThrowVisualSpawnActorInstance;

	//Cheering
	AActor* CheeringItem;
	float CheeringTimeRemaining;

public:
	UPROPERTY(EditDefaultsOnly)
		UPlayerStatsDataAsset* playerData;

	UPROPERTY(EditAnywhere)
		class UInputMappingContext* InputMapping;
	UPROPERTY(EditAnywhere)
		class UInputAction* MoveAction;
	UPROPERTY(EditAnywhere)
		class UInputAction* LookAction;
	UPROPERTY(EditAnywhere)
		class UInputAction* JumpAction;
	UPROPERTY(EditAnywhere)
		class UInputAction* DodgeAction;
	UPROPERTY(EditAnywhere)
		class UInputAction* AttackAction;
	UPROPERTY(EditAnywhere)
		class UInputAction* ThrowSeedAction;

	//Blueprint Functions
	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetAttackSpinUpAlpha();
	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetSpinSpeed();

private:
	void Initialize();
	void UpdateChachedVelocity();
	void UpdateComponentVelocity();
	bool GetGround(FHitResult& HitResult);
	bool GetGround();
	bool GetGroundValidAngle(FHitResult& HitResult);
	bool GetGroundValidAngle();
	void HandleGravity(float Acceleration, float MaxFallSpeed);
	void GroundedCheck();
	void StickToGround();
	void OnGrounded();
	void HandleMove(float AccelerationSpeed, float DecelerationSpeed, float MaxSpeed);
	void HandleGroundedMove(float AccelerationSpeed, float DecelerationSpeed, float MaxSpeed);
	void PointCharacterForwards();
	void PointCharacterTowardCamera();
	FVector GetForwardVector();
	FVector GetRightVector();
	TArray<AEnemyTurret*> CheckForEnemies();
	FVector MoveVectorTowards(FVector current, FVector target, float maxDistanceDelta);
	FRotator GetFlatControlRotation();
	UFUNCTION(BlueprintCallable)
		void TakeDamage(int damage);
	UFUNCTION(BlueprintCallable)
		void RestoreMaxHeatlh();
	UFUNCTION(BlueprintCallable)
		void SetBonusHealth(int Value);
	void Die();
	UFUNCTION(BlueprintCallable)
		void AddRelativeTeleport(FVector Distance);
	UFUNCTION(BlueprintCallable)
		void Teleport(FVector Location);
	void TeleportToLocation();
	void RelativeTeleport();
	UFUNCTION(BlueprintCallable)
		void SetVeloctiy(FVector NewVelocity);
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector GetVeloctiy();
	void HandleWallBounce();
	FVector GetThrowLandingPoint();
	UFUNCTION(BlueprintCallable)
		void RemoveInputForPlayer(bool doPhysics);
	UFUNCTION(BlueprintCallable)
		void ReturnInputForPlayer();
	UFUNCTION(BlueprintCallable)
		void StartCheering(AActor* DisplayActor);
	UFUNCTION(BlueprintCallable)
		void StopCheering();
	UFUNCTION(BlueprintCallable)
		void SetPlayerStaticCameraLocation(FVector Location, FVector ForwardDirection, float Speed);
	UFUNCTION(BlueprintCallable)
		void ReturnPlayerCameraLocation(float Speed);
	UFUNCTION(BlueprintCallable)
		void CharacterLookAt(FVector point);
	void PerfectDodgePerformed();
	bool ValidGroundAngle(FHitResult HitResult);

	// Input
	void MoveInput(const FInputActionValue& Value);
	void CameraLook(const FInputActionValue& Value);
	void JumpPressed();
	void JumpReleased();
	void DodgePressed();
	void DodgeReleased();
	void AttackPressed();
	void AttackReleased();
	void ClearMoveInput();
	void ThrowSeedPressed();
	void ThrowSeedRelease();


	// States
	void IdleEnter();
	void IdleTick();
	void GroundedEnter();
	void GroundedTick();
	void CheckGroundedExitConidtions();
	void EnterJump();
	bool CheckJumpEnter();
	void JumpingTick();
	void CheckJumpExitConidtions();
	void FallingEnter();
	void FallingTick();
	void CheckDodgeEnter();
	void DodgeEnter();
	void DodgeTick();
	void CheckGlideEnter();
	void GlideEnter();
	void GlidingTick();
	void CheckGlideBoostEnter();
	void GlideBoostEnter();
	void GlidingBoostTick();
	void CheckAttackEnter();
	void AttackEnter();
	void AttackTick();
	void StunEnter();
	void StunTick();
	void CheckThrowingSeedEnter();
	void ThrowingSeedEnter();
	void ThrowingSeedTick();
	void CheeringTick();
	void CheeringExit();
	void SlidingEnter();
	void SlidingTick();
	void NoMovementTick();
	void NoInputTick();
};
