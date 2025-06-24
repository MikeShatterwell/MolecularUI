// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/DeveloperSettings.h>

#include "DataProviders/MockStoreDataProvider.h"
#include "MolecularUISettings.generated.h"

/**
 * Static defaults for the MolecularUI plugin.
 */
UCLASS(Config=MolecularUI)
class MOLECULARUI_API UMolecularUISettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "MolecularUI Settings")
	static const UMolecularUISettings* Get();

	UFUNCTION(BlueprintPure, Category = "MolecularUI Settings")
	static TSubclassOf<UObject> GetDefaultStoreDataProviderClass();

	UFUNCTION(BlueprintPure, Category = "MolecularUI Settings")
	static TSoftObjectPtr<UDataTable> GetDefaultStoreItemsDataTable();

	UFUNCTION(BlueprintPure, Category = "MolecularUI Settings")
	static TSoftObjectPtr<UDataTable> GetDefaultOwnedItemsDataTable();

	UFUNCTION(BlueprintPure, Category = "MolecularUI Settings")
	static const FSlateBrush& GetDefaultStoreIcon();
protected:
	UPROPERTY(EditAnywhere, Config, Category = "MolecularUI", meta = (MustImplement = "IStoreViewModelProvider"))
	TSubclassOf<UObject> DefaultStoreDataProviderClass = UMockStoreDataProvider::StaticClass();

	UPROPERTY(EditAnywhere, Config, Category = "MolecularUI")
	TSoftObjectPtr<UDataTable> DefaultStoreItemsDataTable = nullptr;

	UPROPERTY(EditAnywhere, Config, Category = "MolecularUI")
	TSoftObjectPtr<UDataTable> DefaultOwnedItemsDataTable = nullptr;
	
	UPROPERTY(EditAnywhere, Config, Category = "MolecularUI")
	FSlateBrush DefaultStoreIcon = FSlateBrush();
};
