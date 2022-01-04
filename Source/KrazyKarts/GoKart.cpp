// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoKart, ServerState);
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


void AGoKart::UpdateLocationFromVelocty(float DeltaTime)
{

		//v2//translation in meters dx = v * dt
		FVector Translation = (Velocity*100)*DeltaTime;
		//ReplicatedLocation = (Velocity*100)*DeltaTime;

	//в этом случае вызывается конструктор по умолчанию и я могу использовать такую переменную в качестве аут парамтера, но передаю в фю вдрес
	FHitResult OutHitRsult; 
	
	//v2//adding movement to actor, also checking for collision
	AddActorWorldOffset(Translation, true, &OutHitRsult);
	
	if(OutHitRsult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
	
}

void AGoKart::ApplyRotation(FGoKartMove Move)
{
	/*
	 // поврот во времени в направлении
	float RotationAngleDegrees = MaxDegreesPerSecond * DeltaTime * SteeringTrow;
	*/
	
	/// поврот из пройденого пути и радиуса относительно оси вокруг которой происходит пооврот dx = dQ * r
	///  какая часть велсити направлена вперед. Отрицательное - назд, положительное - вперед
	float SpeedInDirection = FVector::DotProduct(GetActorForwardVector(), Velocity);

	/// изменение растсояния во времени. dx
	float DeltaLocation = SpeedInDirection*Move.DeltaTime;

	///dQ = dx/r и умножаем на направление самого поворота     
	float RotationAngleRadians = ( DeltaLocation / RotationCircleRadius)*Move.SteeringTrow;   		
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
	DrawDebugString(GetWorld(), FVector(0, 0, 170), GetName(), this, FColor::Blue, DeltaTime);

	// UE_LOG(LogTemp, Warning, TEXT("%s - isLocallyControlled for %s returns %s"), *GetName(), *GetEnumRole(GetLocalRole()), (IsLocallyControlled() ? TEXT("True") : TEXT("False")));  

	/*
	// на клиенте (AutonomousProxy) создаем структуру и заполняем параметры, затем передаем данные на сервер через PRC
	 
	if(IsLocallyControlled())
	{
		FGoKartMove Move = CreateMove(DeltaTime);

        //IsLocallyControlled() будет тру и для сервера если на нем играет игрок (хостит игру), но Он_Реп там вызываться не будет(репликации для него не будет так как он и так как он играет на сервере на прямую), по этому делаем дополнительный чек так как этот массив нужен только для симуляции на удаленных клиентах
		if(HasAuthority() != true)
		{
			UnacknolegedMoves.Add(Move);
			//UE_LOG(LogTemp,Warning, TEXT("%s On %s Que length %d"),*GetName(), *GetEnumRole(GetRemoteRole()), UnacknolegedMoves.Num());
		}

		if(HasAuthority() != true)
		{
			//server sim, если убрать if b если игрок будет у себя хостить игру(будет играть непрямую на сервере сам), то сенд мув сразу будет срабатывать имплементейшн, где йже прописан СимулейтМув, а потом будет срабатывать второй раз СимулейтМув ниже. По этому тут нужен if который скажет , что эту РПС нужно ыполнять только удаленным клиентам
			Server_SendMove(Move);
		}
		//local sim
		SimulateMove(Move);
		
	}
	*/

	//на удаленном клиенте(см обьяснение  выше там такая же логика только растянутая)  
	if(GetLocalRole() == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = CreateMove(DeltaTime);
		UnacknolegedMoves.Add(Move);
		Server_SendMove(Move);
		SimulateMove(Move);
	}
	
	// на сервере (см обьяснение  выше там такая же логика только растянутая)
	if(GetRemoteRole() == ROLE_SimulatedProxy && GetLocalRole() == ROLE_Authority)
	{
		FGoKartMove Move = CreateMove(DeltaTime);
		Server_SendMove(Move);

	}
	//чтобы избежать прыгания с точки на точку введем плавную симуляция движения серверного павна на клиенте между моментами репликации, 
	if(GetLocalRole() == ROLE_SimulatedProxy)
	{
		SimulateMove(ServerState.LastMove); 
	}

	/*if(GetLocalRole() == ROLE_SimulatedProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("Move Location = %s"), *ServerState.Transform.GetLocation().ToString()); 
	}*/
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
	
	//local sim
	Throttle  = Value;
}

void AGoKart::MoveRight(float Value)
{
	/// если это AutonomousProxy то симуляция происходит локально игроком на своем компе и каждый тик машина знает значение SteeringTrow, но если это SimulatedProxy то это значит что симуляция происходит по средством передачи данных с сервера (а тут может быть лаг), грубо говори когда на сервере происходит MoveRight ф-я, клиент не знает SteeringTrow и он не может
	
	//local sim
	SteeringTrow = Value;
}

void AGoKart::OnRep_ServerState()
{
	//UE_LOG(LogTemp,Warning, TEXT("Replicated location"));

	if(GetLocalRole() == ROLE_SimulatedProxy)
	{
		UE_LOG(LogTemp,Warning, TEXT("Replicated location for ROLE_SimulatedProxy"));
	}

	
	// если Autonomous or Simulated клиент - утанови положение актора из реплицуированой переменной. Слой поверх локальной симуляции. SetActorLocation происходит периодически только на клиенте
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;

	 //когда получили с сервера ServerState, схранить только те локальные стейты которые опрежают серверный стейт   
	ClearAcknoladgedMoves(ServerState.LastMove);

	for(const FGoKartMove& Move : UnacknolegedMoves)
	{
		SimulateMove(Move);
	}
}

void AGoKart::SimulateMove(const FGoKartMove& Move) 
{
		
	///скоуп для локальной и серверной симуляции. Этот кусок будет срабатывать и локально и на сервере. Но реплицируемые пееменные с сервера будут периодически оверрайдить локальную симуляцию.
	{

		//v3// моделирование силы приложенной в какомто направлении
		FVector Force = GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
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

FGoKartMove AGoKart::CreateMove(float DeltaTime)
{
	FGoKartMove Move;
	Move.Throttle = Throttle;
	Move.DeltaTime = DeltaTime;
	Move.SteeringTrow = SteeringTrow;
	Move.Time = GetWorld()->TimeSeconds;

	return Move;
}

void AGoKart::ClearAcknoladgedMoves(FGoKartMove LastMove)
{
	//TSet<FGoKartMove> NewMoves;
	TArray<FGoKartMove> NewMoves;

	for(const FGoKartMove& Move : UnacknolegedMoves)
	{
		if(Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}
	UnacknolegedMoves = NewMoves;
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	SimulateMove(Move); 
	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
	//todo: update last move   

}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Value)
{
	return true;
}
   

