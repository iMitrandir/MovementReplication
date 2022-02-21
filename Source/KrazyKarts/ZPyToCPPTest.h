// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZPyToCPPTest.generated.h"

/**
 * 
 */
UCLASS()
class KRAZYKARTS_API UZPyToCPPTest : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public: 

	UFUNCTION(BlueprintCallable, Category = "Unreal Python")
	static void SimpleLog(FString String);

	UFUNCTION(BlueprintCallable, Category = "Unreal Python")	
	static TArray<FString> GetSelectedAssets();

	/*UFUNCTION(BlueprintCallable, Category = "Unreal Python")	
	static void SetSelectedAssets(TArray<FString> Paths);

	UFUNCTION(BlueprintCallable, Category = "Unreal Python")	
	static TArray<FString> GetSelectedFolders();

	UFUNCTION(BlueprintCallable, Category = "Unreal Python")	
	static void SetSelectedFolders(TArray<FString> Paths);*/
	
};