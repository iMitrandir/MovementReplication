// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.h"

#include "GoKartMoveReplicationComponent.generated.h"

struct FHermitCubicSpline
{
	FVector StartLocation,
			StartDerivative,
			TargetLocation,
			TargetDerivative;

	FVector InterpolateLocation(float LerpRatio) const {return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);};

	FVector InterpolateLocationLinear(float LerpRatio) const {return FMath::LerpStable(StartLocation, TargetLocation, LerpRatio);};
	
	FVector InterpolateDerivative(float LerpRatio) const {return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);}; 
};

USTRUCT()
struct FGoKartState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform Transform;
	
	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FGoKartMove LastMove;
	
};

/// суть работы такая
/// 1. игрок дает инпуты, они отсылаются на сервер и тут же добавляются в массив локально
/// 2. то что пришло на сервер там симулируется, и реплицируется на клиенты.
/// 3. когда пришел ответ с сервера с последним отсимуленым движением, локальный игрок ставится в положение пришедшее с сервера перавм делом
/// 4. затем от этого положения проигрываются все мувы записанные в локальный массив.
/// 5. проигрыш идет до следующей репликации
/// если все было нормально, то значения следующей репликации попадает как раз в рейнж  который находится в массиве для симуляций. Тоесть лаг на клиенте минимально заметен, так как симуляция происходит  в цикле между репликациями, а не в локальном тике.
/// если между клиентом есть несоответвия (к примеру на клиенте обьект упал, а на сервере нет и типа клиент может двигаться дальше, а на сервере он уперся) то при следующей репликации будет принудительный откат на позицию сервера и проигрыш с 0го элемента из массива UnacnowladgedMoves - !!! ОнРеп ф-я вклинивается вработу Тика!!!, когда ОнРеп откатывает изменения положения, Тик заполянет массив от этого нового места, и так п округу

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMoveReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMoveReplicationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
	
	void ClearAcknoladgedMoves(FGoKartMove LastMove);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Value);

	UPROPERTY(ReplicatedUsing=OnRep_ServerState)
	FGoKartState ServerState;

	//TSet<FGoKartMove> UnacknolegedMoves;
	TArray<FGoKartMove> UnacknolegedMoves;
	
	//клиентская ф-я. Апдейтит положение актора на клиенте при репликации структуры FGoKartState с сервера. Локальная симуляция в промежутках между апдейтами идет через МовемнтКомпонент. Эта ф-я накатывает свой результат поверх - фиксит-сенхронизирует сервак при репоикации
	UFUNCTION()
	void OnRep_ServerState();

	void AutonomousProxy_OnRep_ServerState();// ОнРеп выполняемы на айтономойс киеенте
	void SimulatedProxy_OnRep_ServerState();// ОнРеп часть выполняемая на сим прокси
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Movement Replication")	
	UGoKartMovementComponent* GoKartMovementComponent;

	void UpdateServerState(const FGoKartMove& LastMove);

	void Client_Tick(float DelatTime); // ф-я симуляции линейной интераоляции на сим прокси

	// СОЗДАНИЕ СТРУКТУРНОЙ ПЕРМЕННОЙ, сбор данных для формирования сплайна
	FHermitCubicSpline CreateSpline();

	// процесс движение вдоль кривой
	void InterpolateSpline(float LerpRatio, const FHermitCubicSpline& Spline);

	//
	float VelocityToDerivative() {return (Client_TimeBetweenLastUpdates*100.f);};// 100 это перевод из см в метры, так как у нас скорость в м\с, а в анриале расстояние в см 

	// рассчет и установка Velocity исходя из направления движени	
	void InterpolateVelocity(float LerpRatio, const FHermitCubicSpline& Spline);  

	void InterpolateRotation(float LerpRatio);	
	
	float Client_TimeSinceUpdate; // время прошедшее с последего обновления позции на сим прокси
	float Client_TimeBetweenLastUpdates; // время между двумя ОнРеп

	FVector ClientStartVelocity; //

	FTransform Client_StartTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Movement Replication")
	USceneComponent*  MeshOffsetRoot;

	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent*  OffsetRoot) {MeshOffsetRoot = OffsetRoot;};
	
};






