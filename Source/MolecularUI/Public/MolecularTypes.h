// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "MolecularTypes.generated.h"

// Represents the data only the UI would care about for an item.
USTRUCT(BlueprintType)
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

	bool operator==(const FStoreItem& Other) const
	{
		return ItemId == Other.ItemId; // Cost and UIData are not considered for equality
	}
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
	
	bool IsValid() const
	{
		return ItemId != NAME_None;
	}
};

/** Basic interaction events that widgets can choose to emit. */
UENUM(BlueprintType)
enum class EItemInteractionType : uint8
{
	None,
	Hovered,
	Unhovered,
	Clicked,
};

/** Generic stateful channel for communicating simple widget interactions. */
USTRUCT(BlueprintType)
struct FItemInteraction
{
	GENERATED_BODY()

	FItemInteraction() = default;
	explicit FItemInteraction(const EItemInteractionType InType)
			: Type(InType) {}

	/** The latest interaction that occurred. */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	EItemInteractionType Type = EItemInteractionType::None;

	bool operator==(const FItemInteraction& Other) const
	{
		return Type == Other.Type;
	}

	bool IsValid() const
	{
		return Type != EItemInteractionType::None;
	}

	FString ToString() const
	{
		switch (Type)
		{
			case EItemInteractionType::None: return TEXT("None");
			case EItemInteractionType::Hovered: return TEXT("Hovered");
			case EItemInteractionType::Unhovered: return TEXT("Unhovered");
			case EItemInteractionType::Clicked: return TEXT("Clicked");
			default: return TEXT("Unknown");
		}
	}
};