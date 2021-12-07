// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Components/PrimitiveComponent.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
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

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//v3// моделирование силы приложенной в какомто направлении
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	FVector Acceleration = Force / Mass;
															 
	//изменение скорости во времени, учитывая ускорение 
	Velocity = Velocity + Acceleration* DeltaTime; 
	
	UpdateLocationFromVelocty(DeltaTime);

	

}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);	 
}

void AGoKart::MoveForward(float Value)
{
	
	//v1// вариант движения вперед если Велосити это флоат
	/*FVector NewLocation = GetActorLocation() + GetActorForwardVector()* Val * Velocity;
	SetActorLocation(NewLocation);*/

	//v2// на какое расстояние нужно передаивнутся по направлению форвад вектора
	//Velocity = GetActorForwardVector()* Speed * Value;

	//v3// моделирование силы приложенной к массе в какомто направлении
	Throttle  = Value;

}

