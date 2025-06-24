// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Misc/DataValidation.h>

#include "MolecularTypes.generated.h"

// Represents the data only the UI would care about for an item.
USTRUCT(BlueprintType)
struct FItemUIData
{
	GENERATED_BODY()

	FItemUIData() = default;
	FItemUIData(const FText& InDisplayName, const FText& InDescription, const FSlateBrush& InIcon)
		: DisplayName(InDisplayName), Description(InDescription), Icon(InIcon) {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	FText DisplayName = FText::FromString("Default Item");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	FText Description = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	FSlateBrush Icon = FSlateBrush();

	// Add equality operator for MVVM change detection
	bool operator==(const FItemUIData& Other) const
	{
		return DisplayName.EqualTo(Other.DisplayName) 
			&& Description.EqualTo(Other.Description)
			&& Icon == Other.Icon;
	}
};

// Represents a single item available for purchase in the store.
USTRUCT(BlueprintType)
struct FStoreItem : public FTableRowBase
{
	GENERATED_BODY()

	FStoreItem() = default;
	FStoreItem(const FName& InId, const int32 InCost, const bool bInIsOwned, const FItemUIData& InUIData)
		: ItemId(InId), Cost(InCost), bIsOwned(bInIsOwned), UIData(InUIData) {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	int32 Cost = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Store Item")
	bool bIsOwned = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	FItemUIData UIData;

	bool operator==(const FStoreItem& Other) const
	{
		return ItemId == Other.ItemId
			&& Cost == Other.Cost
			&& bIsOwned == Other.bIsOwned
			&& UIData == Other.UIData;
	}

	// Begin FTableRowBase overrides
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override
	{
		if (ItemId == NAME_None)
		{
			Context.AddError(FText::FromString("Store item must have a valid ItemId."));
			return EDataValidationResult::Invalid;
		}
		if (Cost < 0)
		{
			Context.AddError(FText::FromString("Store item cost cannot be negative."));
			return EDataValidationResult::Invalid;
		}
		if (!UIData.DisplayName.IsFromStringTable() && UIData.DisplayName.IsEmpty())
		{
			Context.AddWarning(FText::FromString("Store item UIData DisplayName is empty and not from a string table."));
		}
		if (!UIData.Description.IsFromStringTable() && UIData.Description.IsEmpty())
		{
			Context.AddWarning(FText::FromString("Store item UIData Description is empty and not from a string table."));
		}
		return EDataValidationResult::Valid;
	}
	// End FTableRowBase overrides
};

UENUM(BlueprintType)
enum class ETransactionType : uint8
{
	None,
	Purchase,
	Sell,
};

// Wrapper that represents a user's request to purchase or sell an item.
// This struct is used for the "Stateful Communication" or "Intent Channel".
// A NAME_None ItemId means no request is active.
USTRUCT(BlueprintType)
struct FTransactionRequest
{
	GENERATED_BODY()

	FTransactionRequest() = default;

	UPROPERTY(BlueprintReadWrite, Category = "Transaction Request")
	FName ItemId = NAME_None;

	// We need a custom equality operator for UE_MVVM_SET_PROPERTY_VALUE to work.
	bool operator==(const FTransactionRequest& Other) const
	{
		return ItemId == Other.ItemId;
	}

	bool IsValid() const
	{
		return ItemId != NAME_None;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("TransactionRequest: ItemId=%s"),
							   *ItemId.ToString());
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

/** Wrapper - Generic stateful channel for communicating simple widget interactions for an item. */
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

