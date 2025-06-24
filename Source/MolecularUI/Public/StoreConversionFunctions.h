// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Components/SlateWrapperTypes.h>
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

	UFUNCTION(BlueprintPure, Category = "Store ViewModel|Conversion Functions")
	static ESlateVisibility Conv_StoreStateToVisibility(
		const FGameplayTagContainer& CurrentStoreStates, 
		const FGameplayTag& TestStoreState, 
		const ESlateVisibility HasStateVisibility = ESlateVisibility::Visible, 
		const ESlateVisibility NoStateVisibility = ESlateVisibility::Collapsed);

	UFUNCTION(BlueprintPure, Category = "Store ViewModel|Conversion Functions")
	static ESlateVisibility Conv_StoreTransactionTypeToVisibility(
		const ETransactionType CurrentTransactionType,
		const ETransactionType TestTransactionType = ETransactionType::None,
		const ESlateVisibility TrueVisibility = ESlateVisibility::Visible, 
		const ESlateVisibility FalseVisibility = ESlateVisibility::Collapsed);

	UFUNCTION(BlueprintPure, Category = "Store ViewModel|Conversion Functions")
	static FText Conv_TransactionTypeToText(const ETransactionType TransactionType);

	UFUNCTION(BlueprintPure, Category = "Store ViewModel|Conversion Functions")
	static ESlateVisibility Conv_ObjectIsValidToVisibility(const UObject* Object, 
		const ESlateVisibility ValidVisibility = ESlateVisibility::Visible, 
		const ESlateVisibility InvalidVisibility = ESlateVisibility::Collapsed);
}; 
