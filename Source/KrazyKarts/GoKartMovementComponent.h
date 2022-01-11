// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()
	// поброс маппингов форвард движения из уе. Реплицируются для того чтобы при лагах был известен последннее направление движения,  для правильного подсчета на клиенте, обьекта СимулейтедПрокси сервера.!!! Возморжное место для чита, так как приходит с клиента !!!
	UPROPERTY()
	float Throttle;

	//проброс маппингов поворота из уе. Реплицируются для того чтобы при лагах был известно последннее направление поворота,  для правильного подсчета на клиенте, обьекта СимулейтедПрокси сервера.!!! Возморжное место для чита, так как приходит с клиента !!!  
	UPROPERTY()
	float SteeringTrow;

	UPROPERTY()
	float DeltaTime; //!!! Возморжное место для чита, так как приходит с клиента !!!

	UPROPERTY()
	float Time;

	//cheat protection
	bool IsValid() const {return (Throttle<=1 && SteeringTrow <=1);};
	
	///если буду использвать TSet вместо TArray, раскоменить эти куски
	/*uint32 GetTypeHash() const
	{
		return *(uint32*)&Time;

	}
	
	friend bool operator==(const FGoKartMove& first, const FGoKartMove& second)
	{
		return (first.Time == second.Time);
	}*/
};
///если буду использвать TSet раскоменить эти куски
/*FORCEINLINE uint32 GetTypeHash(const FGoKartMove& other)
{
	return other.GetTypeHash();
}*/


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
	
	void SetThrottle(float Val) {Throttle = Val;};
	void SetSteeringTrow(float Val) {SteeringTrow = Val;};
	
	FVector GetVelocity() const;
	void SetVelocity(FVector NewVelocity) {Velocity = NewVelocity;};

private:

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

	FGoKartMove LastMove;
	
	FGoKartMove CreateMove(float DeltaTime);

	
	
public:
	
	void SimulateMove(const FGoKartMove&);
	FGoKartMove GetLastMove() {return LastMove;};
	
};
