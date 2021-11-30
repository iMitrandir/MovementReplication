// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

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

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//v2//translation in meters
	FVector Translation = (Velocity*100)*DeltaTime;

	//v2//adding movement to actor
	AddActorWorldOffset(Translation);


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
	Velocity = GetActorForwardVector()* 20 * Value;

}

