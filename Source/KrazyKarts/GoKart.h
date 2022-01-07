// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TypeHash.h"

#include "GoKart.generated.h"

class UGoKartMovementComponent;
class UGoKartMoveReplicationComponent;


UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))	
	UGoKartMovementComponent* GoKartMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))	 
	UGoKartMoveReplicationComponent* MovementReplicator;
	
	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()	
	void MoveRight(float Val);

private:
	//тестовая переменная которая чсчитает время - накапливает тик
	float TestTickTime = 0.0;
	
};


