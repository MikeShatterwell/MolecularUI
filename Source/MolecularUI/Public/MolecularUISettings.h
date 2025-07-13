// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/DeveloperSettings.h>

#include "DataProviders/MockStoreDataProviderSubsystem.h"
#include "Models/StoreModel.h"
#include "MolecularUISettings.generated.h"

class UMolecularModelBase;
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
	static TSoftObjectPtr<UDataTable> GetDefaultStoreItemsDataTable();

	UFUNCTION(BlueprintPure, Category = "MolecularUI Settings")
	static const FSlateBrush& GetDefaultStoreIcon();
protected:
	UPROPERTY(EditAnywhere, Config, Category = "MolecularUI")
	TSoftObjectPtr<UDataTable> DefaultItemsDataTable = nullptr;
	
	UPROPERTY(EditAnywhere, Config, Category = "MolecularUI")
	FSlateBrush DefaultStoreIcon = FSlateBrush();
};
