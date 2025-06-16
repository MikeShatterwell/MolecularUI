// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "MolecularTypes.h"
#include "MolecularUITags.h"
#include "StoreConversionFunctions.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MOLECULARUI_API UStoreConversionFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, Category = "Store ViewModel|Conversion Functions")
	static bool Conv_HasStoreState(const FGameplayTagContainer& CurrentStoreStates, const FGameplayTag& TestStoreState);

	UFUNCTION(BlueprintPure, Category = "Store ViewModel|Conversion Functions")
	static FText Conv_StoreStateToText(const FGameplayTagContainer& CurrentStoreStates);
}; 
