// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
}

static FString GetEnumRole (ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None: return  "None";
	case ROLE_SimulatedProxy: return "SimulatedProxy";
	case ROLE_AutonomousProxy: return "AutonomousProxy";
	case ROLE_Authority: return "Authority";
	default: return "Error";
	}
	
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	   
}

void AGoKart::UpdateLocationFromVelocty(float DeltaTime)
{
	//v2//translation in meters dx = v * dt
	FVector Translation = (Velocity*100)*DeltaTime;


	//в этом случае вызывается конструктор по умолчанию и я могу использовать такую переменную в качестве аут парамтера, но передаю в фю вдрес
	FHitResult OutHitRsult; 
	
	//v2//adding movement to actor, also checking for collision
	AddActorWorldOffset(Translation, true, &OutHitRsult);
	
	if(OutHitRsult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
	
	/*
	///////////////////// пример мемори менеджмента //////////////
	//  //создавая указатель и инициализируя его нуллптр я не видляю фактически память под обьект\структуру, по этому запись FHitResult* OutHitRsult=nullptr я не мог использовать как аут параметр - туда ничего не записывалось, а просто брался нулл птр. Но используюя запись через new я выделяю память и могу использваоть ее как аут параметр - это работало.
	
	// 	FHitResult* OutHitRsult= new FHitResult();	
	//
	//  
	//
	// 	//v2//adding movement to actor, also checking for collision
	// 	AddActorWorldOffset(Translation, true, OutHitRsult);
	//
	// 	if(OutHitRsult->IsValidBlockingHit())
	// 	{
	// 		Velocity = FVector::ZeroVector;
	// 	}

	//  //delete OutHitRsult; //для варианта с FHitresult* и new   */
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	/*
	 // поврот во времени в направлении
	float RotationAngleDegrees = MaxDegreesPerSecond * DeltaTime * SteeringTrow;
	*/
	
	/// поврот из пройденого пути и радиуса относительно оси вокруг которой происходит пооврот dx = dQ * r
	///  какая часть велсити направлена вперед. Отрицательное - назд, положительное - вперед
	float SpeedInDirection = FVector::DotProduct(GetActorForwardVector(), Velocity);

	/// изменение растсояния во времени. dx
	float DeltaLocation = SpeedInDirection*DeltaTime;

	///dQ = dx/r и умножаем на направление самого поворота     
	float RotationAngleRadians = ( DeltaLocation / RotationCircleRadius)*SteeringTrow;   

	//угол в кватерн
	const FQuat RotationDelta(GetActorUpVector(), RotationAngleRadians);


	Velocity = RotationDelta.RotateVector(Velocity);
	
	//повроты
	AddActorLocalRotation(RotationDelta);

}

FVector  AGoKart::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ()/ 100;
	float NormalForce = (Mass* AccelerationDueToGravity);
	return -Velocity.GetSafeNormal() * RollResistanceCoefficient * NormalForce;    
	
}
FVector AGoKart::GetAirResistance()
{
	float Speed = Velocity.Size();
	////Velocity.SizeSquared();// == FMath::Square(Speed)      
	float AirResistance = -1  * Velocity.SizeSquared() * DragCoefficient; 

	return Velocity.GetSafeNormal() * AirResistance;
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	  DrawDebugString(GetWorld(), FVector(0, 0, 130), GetEnumRole(GetLocalRole()), this, FColor::Blue, DeltaTime);
	DrawDebugString(GetWorld(), FVector(0, 0, 150), GetEnumRole(GetRemoteRole()), this, FColor::Red, DeltaTime);
	//для тестов
	UE_LOG(LogTemp, Warning, TEXT("%f"), TestTickTime);
	TestTickTime +=DeltaTime;
	
	//v3// моделирование силы приложенной в какомто направлении
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	// учитываем драг форс давление воздуха
	Force = Force + GetAirResistance() + GetRollingResistance();	

	FVector Acceleration = Force / Mass;
	//изменение скорости во времени, учитывая ускорение 
	Velocity = Velocity + Acceleration * DeltaTime;
	
	//повроты
	ApplyRotation(DeltaTime);
	
	// движение вперед
	UpdateLocationFromVelocty(DeltaTime);

}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);

	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float Value)
{
	Server_MoveForward(Value);
	Throttle  = Value;
}

void AGoKart::MoveRight(float Value)
{
	Server_MoveRight(Value);
	SteeringTrow = Value;
}

void AGoKart::Server_MoveForward_Implementation(float Value)
{
	
	//v1// вариант движения вперед если Велосити это флоат
	/*FVector NewLocation = GetActorLocation() + GetActorForwardVector()* Val * Velocity;
	SetActorLocation(NewLocation);*/

	//v2// на какое расстояние нужно передаивнутся по направлению форвад вектора
	//Velocity = GetActorForwardVector()* Speed * Value;

	//v3// моделирование силы приложенной к массе в какомто направлении
	Throttle  = Value;

}

bool AGoKart::Server_MoveForward_Validate(float Value)
{
	return true;
}
   

void AGoKart::Server_MoveRight_Implementation(float Val)
{
	SteeringTrow = Val;
}

bool AGoKart::Server_MoveRight_Validate(float Val)
{

	   return true;
}