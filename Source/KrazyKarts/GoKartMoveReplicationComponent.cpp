// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMoveReplicationComponent.h"
#include "UnrealNetwork.h"
#include "GoKart.h"
#include "GoKartMovementComponent.h"

// Sets default values for this component's properties
UGoKartMoveReplicationComponent::UGoKartMoveReplicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);	 

}


// Called when the game starts
void UGoKartMoveReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	GoKartMovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
	
}

void UGoKartMoveReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGoKartMoveReplicationComponent, ServerState);
}


// Called every frame
void UGoKartMoveReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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

	if(GoKartMovementComponent == nullptr) return;
	
	//на удаленном клиенте(см обьяснение  выше там такая же логика только растянутая)  
	if(GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = GoKartMovementComponent->CreateMove(DeltaTime);
		UnacknolegedMoves.Add(Move);
		Server_SendMove(Move);
		GoKartMovementComponent->SimulateMove(Move);
	}
	
	// на сервере (см обьяснение  выше там такая же логика только растянутая)
	if(GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy && GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		FGoKartMove Move = GoKartMovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move);

	}
	//чтобы избежать прыгания с точки на точку введем плавную симуляция движения серверного павна на клиенте между моментами репликации, 
	if(GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		GoKartMovementComponent->SimulateMove(ServerState.LastMove); 
	}

	/*if(GetLocalRole() == ROLE_SimulatedProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("Move Location = %s"), *ServerState.Transform.GetLocation().ToString()); 
	}*/
}

void UGoKartMoveReplicationComponent::ClearAcknoladgedMoves(FGoKartMove LastMove)
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

void UGoKartMoveReplicationComponent::Server_SendMove_Implementation(FGoKartMove Move)
{
	if(GoKartMovementComponent == nullptr) return;
	
	GoKartMovementComponent->SimulateMove(Move); 
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = GoKartMovementComponent->GetVelocity();

}

bool UGoKartMoveReplicationComponent::Server_SendMove_Validate(FGoKartMove Value)
{
	return true;
}

void UGoKartMoveReplicationComponent::OnRep_ServerState()
{
	//UE_LOG(LogTemp,Warning, TEXT("Replicated location"));
	if(GoKartMovementComponent == nullptr) return;    
	if(GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		UE_LOG(LogTemp,Warning, TEXT("Replicated location for ROLE_SimulatedProxy"));
	}

	
	// если Autonomous or Simulated клиент - утанови положение актора из реплицуированой переменной. Слой поверх локальной симуляции. SetActorLocation происходит периодически только на клиенте
	GetOwner()->SetActorTransform(ServerState.Transform);
	GoKartMovementComponent->SetVelocity(ServerState.Velocity);

	//когда получили с сервера ServerState, схранить только те локальные стейты которые опрежают серверный стейт   
	ClearAcknoladgedMoves(ServerState.LastMove);

	for(const FGoKartMove& Move : UnacknolegedMoves)
	{
		GoKartMovementComponent->SimulateMove(Move);
	}
}

