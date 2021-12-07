// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

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

	void UpdateLocationFromVelocty(float DeltaTime);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
	UFUNCTION()
	void MoveForward(float Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
	float Speed = 20.f;

	//Mass of object in (kg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
	float Mass = 1000.f;

	//максимальная сила которая может быть приложена к средству (N)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
	float MaxDrivingForce = 10000.f;

private:
	FVector Velocity;

	float Throttle;

};
