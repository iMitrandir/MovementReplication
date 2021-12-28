// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()
	// поброс маппингов форвард движения из уе. Реплицируются для того чтобы при лагах был известен последннее направление движения,  для правильного подсчета на клиенте, обьекта СимулейтедПрокси сервера.
	UPROPERTY()
	float Throttle;

	//проброс маппингов поворота из уе. Реплицируются для того чтобы при лагах был известно последннее направление поворота,  для правильного подсчета на клиенте, обьекта СимулейтедПрокси сервера.
	UPROPERTY()
	float SteeringTrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;	
};

USTRUCT()
struct FGoKartState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform Transform;
	
	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FGoKartMove LastMove;
	
};

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateLocationFromVelocty(float DeltaTime);
	void ApplyRotation(float DeltaTime);

	//силы действующие на авто против движеня 
	FVector  GetAirResistance();
	FVector  GetRollingResistance();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()	
	void MoveRight(float Val);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Value);
	

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
	// float Speed = 20.f;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
	float MaxDegreesPerSecond = 90.f;*/

	//Mass of object in (kg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
	float Mass = 1000.f;

	//максимальная сила которая может быть приложена к средству (N)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
	float MaxDrivingForce = 10000.f;

	//коефициент аэоодинамики, чем он больше, тем больше будет AirResistance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
	float DragCoefficient = 16.f;

	//коефициент трения, чем он больше, тем больше будет RollResistance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
	float RollResistanceCoefficient = 0.015f;

	//радиус поворота мошины
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
	float RotationCircleRadius = 10.f;

	UPROPERTY(ReplicatedUsing=OnRep_ServerState)
	FGoKartState ServerState;

	UFUNCTION()
	void OnRep_ServerState();



private:
	
	// поброс маппингов форвард движения из уе. Реплицируются для того чтобы при лагах был известен последннее направление движения,  для правильного подсчета на клиенте, обьекта СимулейтедПрокси сервера.
	UPROPERTY(Replicated)
	float Throttle;

	//проброс маппингов поворота из уе. Реплицируются для того чтобы при лагах был известно последннее направление поворота,  для правильного подсчета на клиенте, обьекта СимулейтедПрокси сервера.
	UPROPERTY(Replicated)
	float SteeringTrow;

	//тестовая переменная которая чсчитает время - накапливает тик
	float TestTickTime = 0.0;
	

	// положение на клиенте в момент старата передачи на сервер (в первый момент активности игрока)  
	FTransform CachedReplicatedTransform;

	// клиентская локальная симуляция
	FVector Velocity;
	private:

	void SimulateMove(FGoKartMove);
	                   
	
};


