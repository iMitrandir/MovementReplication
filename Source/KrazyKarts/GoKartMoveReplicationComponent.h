// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.h"

#include "GoKartMoveReplicationComponent.generated.h"

/*struct FGoKartMove;
struct FGoKartState;*/
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
	
	//клиентская ф-я. Апдейтит положение актора на клиенте при репликации структуры FGoKartState с сервера
	UFUNCTION()
	void OnRep_ServerState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))	
	UGoKartMovementComponent* GoKartMovementComponent;
		
};
