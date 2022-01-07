// Fill out your copyright notice in the Description page of Project Settings.



#include "GoKartMovementComponent.h"
#include "GoKart.h"

// Sets default values for this component's properties
UGoKartMovementComponent::UGoKartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGoKartMovementComponent::UpdateLocationFromVelocty(float DeltaTime)
{
	
	//v2//translation in meters dx = v * dt
	FVector Translation = (Velocity*100)*DeltaTime;
	//ReplicatedLocation = (Velocity*100)*DeltaTime;

	//в этом случае вызывается конструктор по умолчанию и я могу использовать такую переменную в качестве аут парамтера, но передаю в фю вдрес
	FHitResult OutHitRsult; 
	
	//v2//adding movement to actor, also checking for collision
	GetOwner()->AddActorWorldOffset(Translation, true, &OutHitRsult);
	
	if(OutHitRsult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
	
}

void UGoKartMovementComponent::ApplyRotation(FGoKartMove Move)
{
	/*
	 // поврот во времени в направлении
	float RotationAngleDegrees = MaxDegreesPerSecond * DeltaTime * SteeringTrow;
	*/
	
	/// поврот из пройденого пути и радиуса относительно оси вокруг которой происходит пооврот dx = dQ * r
	///  какая часть велсити направлена вперед. Отрицательное - назд, положительное - вперед
	float SpeedInDirection = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity);

	/// изменение растсояния во времени. dx
	float DeltaLocation = SpeedInDirection * Move.DeltaTime;

	///dQ = dx/r и умножаем на направление самого поворота     
	float RotationAngleRadians = ( DeltaLocation / RotationCircleRadius)*Move.SteeringTrow;   		
	//угол в кватерн
	const FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngleRadians);

	Velocity = RotationDelta.RotateVector(Velocity);
	
	//повроты
	GetOwner()->AddActorLocalRotation(RotationDelta);
}

FVector UGoKartMovementComponent::GetAirResistance()
{
	float Speed = Velocity.Size();
	////Velocity.SizeSquared();// == FMath::Square(Speed)      
	float AirResistance = -1  * Velocity.SizeSquared() * DragCoefficient; 

	return Velocity.GetSafeNormal() * AirResistance;
}

FVector UGoKartMovementComponent::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ()/ 100;
	float NormalForce = (Mass * AccelerationDueToGravity);
	return -Velocity.GetSafeNormal() * RollResistanceCoefficient * NormalForce;  
}

void UGoKartMovementComponent::SimulateMove(const FGoKartMove& Move)
{
	///скоуп для локальной и серверной симуляции. Этот кусок будет срабатывать и локально и на сервере. Но реплицируемые пееменные с сервера будут периодически оверрайдить локальную симуляцию.
	{

		//v3// моделирование силы приложенной в какомто направлении
		FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
		// учитываем драг форс давление воздуха
		Force = Force + GetAirResistance() + GetRollingResistance();	

		FVector Acceleration = Force / Mass;

		//изменение скорости во времени, учитывая ускорение 
		Velocity = Velocity + Acceleration * Move.DeltaTime;      
		
		//повроты
		ApplyRotation(Move);
		
		// движение вперед
		UpdateLocationFromVelocty(Move.DeltaTime);
	}
}

FGoKartMove UGoKartMovementComponent::CreateMove(float DeltaTime)
{
	FGoKartMove Move;
	Move.Throttle = Throttle;
	Move.DeltaTime = DeltaTime;
	Move.SteeringTrow = SteeringTrow;
	Move.Time = GetWorld()->TimeSeconds;

	return Move;
}

