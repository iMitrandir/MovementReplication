// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"

#include "GoKartMoveReplicationComponent.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GoKartMovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(FName("GoKartMovementComponent"));

	MovementReplicator = CreateDefaultSubobject<UGoKartMoveReplicationComponent>(FName("GoKartMovementReplicator"));
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
	
	//просимулируем разные интервалы для репликации (раз в секунду)
	if(HasAuthority())
	{
		NetUpdateFrequency = 1.f;
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	
	Super::Tick(DeltaTime);
	
	DrawDebugString(GetWorld(), FVector(0, 0, 130), GetEnumRole(GetLocalRole()), this, FColor::Blue, DeltaTime);
	DrawDebugString(GetWorld(), FVector(0, 0, 150), GetEnumRole(GetRemoteRole()), this, FColor::Red, DeltaTime);
	DrawDebugString(GetWorld(), FVector(0, 0, 170), GetName(), this, FColor::Blue, DeltaTime);

	// UE_LOG(LogTemp, Warning, TEXT("%s - isLocallyControlled for %s returns %s"), *GetName(), *GetEnumRole(GetLocalRole()), (IsLocallyControlled() ? TEXT("True") : TEXT("False")));  

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
	if(GoKartMovementComponent == nullptr) return; 
	//local sim
	GoKartMovementComponent->SetThrottle(Value);
}

void AGoKart::MoveRight(float Value)
{
	if(GoKartMovementComponent == nullptr) return;
	
	/// если это AutonomousProxy то симуляция происходит локально игроком на своем компе и каждый тик машина знает значение SteeringTrow, но если это SimulatedProxy то это значит что симуляция происходит по средством передачи данных с сервера (а тут может быть лаг), грубо говори когда на сервере происходит MoveRight ф-я, клиент не знает SteeringTrow и он не может
	
	//local sim
	GoKartMovementComponent->SetSteeringTrow(Value);
}





