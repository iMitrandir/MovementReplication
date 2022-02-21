// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CppHelpers.generated.h"

/**
 * 
 */
UCLASS()
class CPPTOPYHELPERS_API UCppHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
public:
	UFUNCTION(BlueprintCallable, Category = "Unreal Python")
		static TArray<FString> GetAllProperties(UClass* Class);

	UFUNCTION(BlueprintCallable, Category = "Unreal Python")
		static void ExecuteConsoleCommand(FString ConsoleCommand);
	
	
};
