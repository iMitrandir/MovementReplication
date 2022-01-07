// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TypeHash.h"

#include "GoKart.generated.h"

class UGoKartMovementComponent;

USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()
	// поброс маппингов форвард движения из уе. Реплицируются для того чтобы при лагах был известен последннее направление движения,  для правильного подсчета на клиенте, обьекта СимулейтедПрокси сервера.
	UPROPERTY()
	float Throttle;

	//проброс маппингов поворота из уе. Реплицируются для того чтобы при лагах был известно последннее направление поворота,  для правильного подсчета на клиенте, обьекта СимулейтедПрокси сервера.
	UPROPERTY()
	float SteeringTrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;

	///если буду использвать TSet вместо TArray, раскоменить эти куски
	/*uint32 GetTypeHash() const
	{
		return *(uint32*)&Time;

	}
	
	friend bool operator==(const FGoKartMove& first, const FGoKartMove& second)
	{
		return (first.Time == second.Time);
	}*/
};
///если буду использвать TSet раскоменить эти куски
/*FORCEINLINE uint32 GetTypeHash(const FGoKartMove& other)
{
	return other.GetTypeHash();
}*/

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

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))	
	UGoKartMovementComponent* GoKartMovementComponent;

protected:
	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()	
	void MoveRight(float Val);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Value);
	
	UPROPERTY(ReplicatedUsing=OnRep_ServerState)
	FGoKartState ServerState;

private:
	
	//тестовая переменная которая чсчитает время - накапливает тик
	float TestTickTime = 0.0;
	
	//TSet<FGoKartMove> UnacknolegedMoves;
	TArray<FGoKartMove> UnacknolegedMoves;

	void ClearAcknoladgedMoves(FGoKartMove LastMove);

	//клиентская ф-я. Апдейтит положение актора на клиенте при репликации структуры FGoKartState с сервера
	UFUNCTION()
	void OnRep_ServerState();	                   
	
};


