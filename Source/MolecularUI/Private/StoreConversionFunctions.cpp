// Copyright Mike Desrosiers, All Rights Reserved.

#include "StoreConversionFunctions.h"
#include "MolecularUITags.h"

bool UStoreConversionFunctions::Conv_HasStoreState(const FGameplayTagContainer& CurrentStoreStates,
												   const FGameplayTag& TestStoreState)
{
	return CurrentStoreStates.HasTagExact(TestStoreState);
}

FText UStoreConversionFunctions::Conv_StoreStateToText(const FGameplayTagContainer& CurrentStoreStates)
{
	
	FText ResultText;

	if (CurrentStoreStates.Num() == 0)
	{
		return FText::FromString("No States");
	}

	// Map to store counts of each tag
	TMap<FName, int32> TagCounts;
	for (const FGameplayTag& Tag : CurrentStoreStates)
	{
		TagCounts.FindOrAdd(Tag.GetTagName())++;
	}

	// Build the output string
	FString ResultString;
	for (const auto& TagCountPair : TagCounts)
	{
		if (!ResultString.IsEmpty())
		{
			ResultString.Append(", ");
		}

		ResultString.Append(FString::Printf(TEXT("%s (x%d)"), *TagCountPair.Key.ToString(), TagCountPair.Value));
	}

	return FText::FromString(ResultString);
}

ESlateVisibility UStoreConversionFunctions::Conv_StoreStateToVisibility(const FGameplayTagContainer& CurrentStoreStates,
	const FGameplayTag& TestStoreState, const ESlateVisibility HasStateVisibility,
	const ESlateVisibility NoStateVisibility)
{
	return CurrentStoreStates.HasTagExact(TestStoreState) ? HasStateVisibility : NoStateVisibility;
}

ESlateVisibility UStoreConversionFunctions::Conv_StoreTransactionTypeToVisibility(
	const ETransactionType CurrentTransactionType, const ETransactionType TestTransactionType,
	const ESlateVisibility TrueVisibility, const ESlateVisibility FalseVisibility)
{
	return CurrentTransactionType == TestTransactionType ? TrueVisibility : FalseVisibility;
}

FText UStoreConversionFunctions::Conv_TransactionTypeToText(const ETransactionType TransactionType)
{
	// Demo only, this could be replaced with a more comprehensive mapping or localization system.
	switch (TransactionType)
	{
	case ETransactionType::None:
		return FText::FromString("None");
	case ETransactionType::Purchase:
		return FText::FromString("Purchase");
	case ETransactionType::Sell:
		return FText::FromString("Sell");
	default:
		return FText::FromString("Unknown");
	}
}

ESlateVisibility UStoreConversionFunctions::Conv_ObjectIsValidToVisibility(const UObject* Object,
	const ESlateVisibility ValidVisibility, const ESlateVisibility InvalidVisibility)
{
	return IsValid(Object) ? ValidVisibility : InvalidVisibility;
}
