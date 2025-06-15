// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "MolecularTypes.h"
#include "StoreConversionFunctions.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MOLECULARUI_API UStoreConversionFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, Category = "Store ViewModel|Conversion Functions")
	static bool Conv_AreStoreStatesEqual(const EStoreState CurrentStoreState, const EStoreState TestStoreState);

	UFUNCTION(BlueprintPure, Category = "Store ViewModel|Conversion Functions")
	static FText Conv_StoreStateToText(const EStoreState StoreState);
};
