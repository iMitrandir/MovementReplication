// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMoveReplicationComponent.h"
#include "UnrealNetwork.h"
#include "GoKart.h"
#include "GoKartMovementComponent.h"
#include "Math/UnrealMathVectorCommon.h"

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
	//MeshOffsetRoot = GetOwner()->FindComponentByClass<USceneComponent>();
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
	
	if(GoKartMovementComponent == nullptr) return;

	FGoKartMove LastMove = GoKartMovementComponent->GetLastMove();
	//Клиент симулирует последовательность передвижений с опережением сервера, но считывает эту последовательность не прямо из инпута, а из массива структур. По ходу дела Создаем на удаленном клиенте. Массив структур хранит последовательность передвижений на клиенте, для послудующего сравнения с серверным последним движением отправленым на клиент. После сравнения на клиенте, все мувы которые старые (просимулированы на серве, тоесть UnAccnowledgedMoves[i]<ServerLastMove) отбрасываются, а мувы которые еще не просимулированы на сервере UnAccnowledgedMoves[i]>ServerLastMove проигрываются на клиенте(циклом в Simulate). Сравнение происходит с помошью временого штампа. Тоесть типа приходит на клиент сообщение, что на сервере был проигран мув с временным штампом Х, по этому нужно удалить все мувы временные штампы которых меньше Х, но проиграть все мувы временные штампы которых больше Х. Грубо говоря инпут записывается в буфер, а потом проигрывается или же вносятся поправки если в течении лага, что-то кардианально поменялось в синхронизации клиента и сервера.  
	if(GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		//UE_LOG(LogTemp, Warning, TEXT("UnacknolegedMoves = %d"), UnacknolegedMoves.Num());
		UnacknolegedMoves.Add(LastMove);
		Server_SendMove(LastMove);

	}
	
	// на сервере (см обьяснение  выше там такая же логика только растянутая)
	if(GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy && GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		UpdateServerState(LastMove);
	}

	if(GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		/*
		 * 		//движения без интерполяции, а просто скачками при высоком лагедля сим прокси 
		//UE_LOG(LogTemp, Warning, TEXT("Move Location = %s"), *ServerState.Transform.GetLocation().ToString()); 

		//GoKartMovementComponent->SimulateMove(ServerState.LastMove);
		*/

		// плавная интерполяция между 2мя точками для сим прокси. Но при высоком лаге не успевает за сервером
		Client_Tick(DeltaTime);
	}
	 
}

void UGoKartMoveReplicationComponent::Client_Tick(float DeltaTime)
{
	Client_TimeSinceUpdate+=DeltaTime;

	//проверка на то что  TimeBetweenLastUpdates не является слишкоммаленьким числом близким к 0, если это не так - значит это первый апдейт, его пропускаем. 
	if(Client_TimeBetweenLastUpdates == KINDA_SMALL_NUMBER) return;
	if(GoKartMovementComponent == nullptr) {return;}
	
	float LerpRatio = Client_TimeSinceUpdate/Client_TimeBetweenLastUpdates;

	FHermitCubicSpline Spline =  CreateSpline();

	/// интерполяция движения - пример линейной интерполЯции 
	/*FVector NewLocation = InterpolateLocationLinear(LerpRatio);// linear interp
	GetOwner()->SetActorLocation(NewLocation);*/

	InterpolateSpline(LerpRatio, Spline);
    	
	InterpolateVelocity(LerpRatio, Spline);
	
	InterpolateRotation(LerpRatio);
}

FHermitCubicSpline UGoKartMoveReplicationComponent::CreateSpline()
{
	//	проигрывание  интерполяции в тике для SimProx
	FHermitCubicSpline Spline;
	Spline.StartLocation = Client_StartTransform.GetLocation();
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	
	Spline.StartDerivative = (ClientStartVelocity * VelocityToDerivative());
	Spline.TargetDerivative = ServerState.Velocity * VelocityToDerivative();
	return Spline; 
	
}

void UGoKartMoveReplicationComponent::InterpolateSpline(float LerpRatio, const FHermitCubicSpline& Spline)
{
	FVector NewLocation = Spline.InterpolateLocation(LerpRatio);
	MeshOffsetRoot->SetWorldLocation(NewLocation);
}

void UGoKartMoveReplicationComponent::InterpolateVelocity(float LerpRatio, const FHermitCubicSpline& Spline)
{
	// рассчет и установка Velocity исходя из направления движени
	FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);//
	FVector NewVelocity = NewDerivative / VelocityToDerivative();

	GoKartMovementComponent->SetVelocity(NewVelocity);
}

void UGoKartMoveReplicationComponent::InterpolateRotation(float LerpRatio)
{
	// проигрвыние интерполяции поворота в тике для SimProx
	MeshOffsetRoot->SetWorldRotation(FQuat::Slerp(FQuat(Client_StartTransform.GetRotation()), FQuat(ServerState.Transform.GetRotation()), LerpRatio));
}

void UGoKartMoveReplicationComponent:: ClearAcknoladgedMoves(FGoKartMove LastMove)
{
	//TSet<FGoKartMove> NewMoves;
	TArray<FGoKartMove> NewMoves; // мувы совершенные на клиенте, но еще не просимулированные на сервере

	//мувы которые опережают мув пришедший с серва, сохраняем чтобы потом просимулировать
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
	Client_SimulatedTimeCounter	+= Move.DeltaTime;

	if(GoKartMovementComponent == nullptr) return;

	GoKartMovementComponent->SimulateMove(Move); 

	UpdateServerState(Move);
}

bool UGoKartMoveReplicationComponent::Server_SendMove_Validate(FGoKartMove Value)
{
	float ProposedTime = Client_SimulatedTimeCounter + Value.DeltaTime;
	///смысл в том, что в тике может приходить несколько одинаковых РПСб в этом случае локальный таймер сервера не будет изменяться (так как это в рамках одного тика РПС), но информация в сообщениях можо настакать. Суть такая же как если умножить просто какой-то парамтер на число, но в этом случае идет настакивание в одном тике
	///Сравнить неизменный парамтер сервера с настаканными парамтерами из РПС 
	bool ClientNotRunningAhead = ProposedTime > GetWorld()->TimeSeconds;
	if(ClientNotRunningAhead)
	{
		UE_LOG(LogTemp,Error,TEXT("Client running too fast"));
		return false;
	}
	return Value.IsValid();
}

void UGoKartMoveReplicationComponent::OnRep_ServerState()
{
	switch (GetOwner()->GetLocalRole())
	{
	case ROLE_SimulatedProxy: SimulatedProxy_OnRep_ServerState();
		break;
	case ROLE_AutonomousProxy:AutonomousProxy_OnRep_ServerState(); 
		break;

	default:
		break;
	}

}

void UGoKartMoveReplicationComponent::AutonomousProxy_OnRep_ServerState()
{
	//UE_LOG(LogTemp,Warning, TEXT("Replicated location"));
	if(GoKartMovementComponent == nullptr) return;    
	if(GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		//UE_LOG(LogTemp,Warning, TEXT("Replicated location for ROLE_SimulatedProxy"));
	}
	
	// на Autonomous or Simulated клиент - утанови положение актора из реплицуированой переменной, синхронизирует поворот. Слой поверх локальной симуляции. SetActorLocation происходит периодически только на клиентах.
	//тоесть вначале синхронизируется положение с сервером, а потом проигрывается из откорректированого массива то что было отсимулировано на клиенте с этого момента
	GetOwner()->SetActorTransform(ServerState.Transform);
	GoKartMovementComponent->SetVelocity(ServerState.Velocity);
	

	//обоновляет UnacknolegedMoves массив. когда получили с сервера ServerState (сервер реплицировал на клиенты изменившиеся данные), схранить только те локальные стейты которые опрежают серверный стейт. Нужно для того чтобы массив АнакновледжедМувс не рос, а постоянно удалались устаревшие локальные движения, 
	ClearAcknoladgedMoves(ServerState.LastMove);

	
	//для AutonomousProxy -  проигрывание происходило от последнего  пришедшего с сервера Мува и дальше по записанным локально Мувам   
	for(const FGoKartMove& Move : UnacknolegedMoves)
	{
		GoKartMovementComponent->SimulateMove(Move);
	}
}

void UGoKartMoveReplicationComponent::SimulatedProxy_OnRep_ServerState()
{
	if(GoKartMovementComponent == nullptr) return;
	
	Client_TimeBetweenLastUpdates = Client_TimeSinceUpdate; // только получили новый апдейт. и время между пердыдущим (при условии что подсчет ведется от 0) апдейтом и текущим-полученным будет = натиканому таймеру  

	Client_TimeSinceUpdate = 0; // так как получили новый апдейт, сбросим натиканный таймер в 0, чтобы увеличивать время от нуля до след апдейта

	//Client_StartTransform = GetOwner()->GetActorTransform();
	Client_StartTransform = MeshOffsetRoot->GetComponentTransform();	 

	//ClientStartVelocity = GoKartMovementComponent->GetVelocity();
	ClientStartVelocity = ServerState.Velocity;

	GetOwner()->SetActorTransform(ServerState.Transform);
	//UE_LOG(LogTemp,Warning, TEXT("Velocity vec len = %f"),		GoKartMovementComponent->GetVelocity().Size()); 
}

void UGoKartMoveReplicationComponent::UpdateServerState(const FGoKartMove& LastMove)
{
	ServerState.LastMove = LastMove;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = GoKartMovementComponent->GetVelocity();
}

