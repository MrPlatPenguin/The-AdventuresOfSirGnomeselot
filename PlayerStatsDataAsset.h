// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/ObjectMacros.h"
#include "Curves/CurveFloat.h"
#include "PlayerStatsDataAsset.generated.h"

/**
 *
 */
UCLASS()
class GARDENGAME_API UPlayerStatsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
		float BaseMoveSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
		float BaseMoveAcceleration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
		float BaseMoveDeceleration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
		float MaxGroundSlopeAngle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
		float GroundingDistance;

	// Camera
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
		float CameraVerticalSensitivity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
		float CameraHorizontalSensitivity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
		float CameraDistance;

	// Jumping
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jumping")
		float FallAcceleration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jumping")
		float MaxFallSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jumping")
		float JumpForce;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jumping")
		float MinJumpHoldTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jumping")
		float MaxJumpHoldTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jumping")
		float FallHorizontalAcceleration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jumping")
		float FallHorizontalDeceleration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jumping")
		float JumpBufferWindow;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jumping")
		float CoyotteTime;

	// Gliding
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding")
		float MaxGlideFallSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding")
		float GlideHorizontalAcceleration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding")
		float GlideHorizontalDeceleration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding")
		float GlideMoveSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding")
		float BoostAcceleration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding")
		float MaxGlideBoostSpeed;

	//Dodge
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
		float DodgeDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
		float DodgeSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
		FRuntimeFloatCurve DodgeSpeedCurve;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
		float PerfectDodgeWindow;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
		float DodgeSlowSpinFactor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
		float PerfectDodgeSlowMotionFactor;


	// Attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float AttackRange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		FRuntimeFloatCurve AttackSpinUpCurve;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float SpinUpTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float AttackingMoveAcceleration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float AttackingMoveDeceleration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float AttackingMoveSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float WallBounceCheckDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float WallBounceForce;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float WallBounceAngle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float WallBounceSpeedReductionFactor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float MaxRotationSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		int StartingHealth;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		float StunTime;

	// Planting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Planting")
		float PlantingThrowRange;

	// Planting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheering")
		float CheeringDuration;
};
