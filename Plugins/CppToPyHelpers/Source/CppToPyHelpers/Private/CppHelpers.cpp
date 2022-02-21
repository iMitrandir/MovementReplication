// Fill out your copyright notice in the Description page of Project Settings.


#include "CppHelpers.h"



TArray<FString> UCppHelpers::GetAllProperties(UClass* Class)
{
	TArray<FString> PropNames;
	if (Class != nullptr) {
		for (TFieldIterator<FProperty> It(Class); It; ++It) {
			FProperty* Property = *It;
			if (Property->HasAnyPropertyFlags(EPropertyFlags::CPF_Edit)) {
				PropNames.Add(Property->GetName());
			}
		}
	}
	return PropNames;
}

void UCppHelpers::ExecuteConsoleCommand(FString ConsoleCommand)
{
	if (GEditor) {
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (World) {
			GEditor->Exec(World, *ConsoleCommand, *GLog);
		}
	}
}

