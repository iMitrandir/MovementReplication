// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

private:

	void SimulateMove(const FGoKartMove&);

	FGoKartMove CreateMove(float DeltaTime);

	void UpdateLocationFromVelocty(float DeltaTime);
	void ApplyRotation(FGoKartMove Move);

	//силы действующие на авто против движеня 
	FVector  GetAirResistance();
	FVector  GetRollingResistance();

	// маппинги направления дыижения для локальной симуляции. На сервер посылается структура содержашая в себе такие же парамтеры.
	UPROPERTY()
	float Throttle;

	// маппинги направления дыижения для локальной симуляции. На сервер посылается структура содержашая в себе такие же парамтеры.
	UPROPERTY()
	float SteeringTrow;

	// клиентская локальная симуляция
	FVector Velocity;

	
};
