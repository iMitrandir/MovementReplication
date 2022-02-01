// Fill out your copyright notice in the Description page of Project Settings.


#include "ZPyToCPPTest.h"

void UZPyToCPPTest::SimpleLog(FString String)
{
	UE_LOG(LogTemp, Warning, TEXT("Py to cpp says : %s"), *String);
}
