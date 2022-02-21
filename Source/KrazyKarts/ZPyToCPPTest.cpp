// Fill out your copyright notice in the Description page of Project Settings.


#include "ZPyToCPPTest.h"


/*#include "ContentBrowserModule.h"
#include "Editor/ContentBrowser/Private/SContentBrowser.h" // not woerking ob 4.25 and higher - need custom engine buid
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "Editor/EditorWidgets/Public/SAssetSearchBox.h"*/


void UZPyToCPPTest::SimpleLog(FString String)
{
	UE_LOG(LogTemp, Warning, TEXT("Py to cpp says : %s"), *String);
}

/*TArray<FString> UZPyToCPPTest::GetSelectedAssets()
{
	UE_LOG(LogTemp, Warning, TEXT("--------Getting the list of selected assets on CB-------------"));
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);
	TArray<FString> Results;
	for(auto& AssetData : SelectedAssets)
	{
		Results.Add(AssetData.PackageName.ToString());
	}

	return Results;
}*/

/*
void UZPyToCPPTest::SetSelectedAssets(TArray<FString> Paths)
{
}

TArray<FString> UZPyToCPPTest::GetSelectedFolders()
{
}

void UZPyToCPPTest::SetSelectedFolders(TArray<FString> Paths)
{
}
*/
