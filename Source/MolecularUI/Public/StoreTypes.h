// Copyright Shatterwell Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StoreTypes.generated.h"

UENUM()
enum class EStoreState : uint8
{
	None,          // No state set
	Loading,       // Loading data
	Ready,         // Ready for interaction
	Purchasing,    // In the process of purchasing an item
	Error          // An error occurred during the process
};

// Represents the data only the UI would care about for an item.
USTRUCT()
struct FItemUIData
{
	GENERATED_BODY()

	FItemUIData() = default;
	FItemUIData(const FText& InDisplayName)
		: DisplayName(InDisplayName) {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	FText DisplayName = FText::FromString("Default Item");
};

// Represents a single item available for purchase in the store.
USTRUCT(BlueprintType)
struct FStoreItem
{
	GENERATED_BODY()

	FStoreItem() = default;
	FStoreItem(const FName& InId, const int32 InCost, const FItemUIData& InUIData)
		: ItemId(InId), Cost(InCost), UIData(InUIData) {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	int32 Cost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	FItemUIData UIData;
};

// Represents a user's request to purchase an item.
// This struct is used for the "Stateful Communication" or "Intent Channel".
// A NAME_None ItemId means no request is active.
USTRUCT(BlueprintType)
struct FPurchaseRequest
{
	GENERATED_BODY()

	FPurchaseRequest() = default;

	UPROPERTY(BlueprintReadWrite, Category = "Purchase Request")
	FName ItemId = NAME_None;

	// We need a custom equality operator for UE_MVVM_SET_PROPERTY_VALUE to work.
	bool operator==(const FPurchaseRequest& Other) const
	{
		return ItemId == Other.ItemId;
	}
};