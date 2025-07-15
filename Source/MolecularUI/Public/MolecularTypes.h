// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <Misc/DataValidation.h>

#include "MolecularUITags.h"
#include "MolecularTypes.generated.h"

// Represents common data only the UI cares about.
USTRUCT(BlueprintType)
struct FStandardUIData
{
	GENERATED_BODY()

	FStandardUIData() = default;
	FStandardUIData(const FText& InDisplayName, const FText& InDescription, const FSlateBrush& InIcon)
		: DisplayName(InDisplayName), Description(InDescription), Icon(InIcon) {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName = FText::FromString("Display Name");

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSlateBrush Icon = FSlateBrush();

	// Add equality operator for MVVM change detection
	bool operator==(const FStandardUIData& Other) const
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
	FStoreItem(const FName& InId, const int32 InCost, const bool bInIsOwned, const FStandardUIData& InUIData, const FGameplayTagContainer& InCategories)
		: ItemId(InId), Cost(InCost), bIsOwned(bInIsOwned), UIData(InUIData), Categories(InCategories) {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	int32 Cost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	bool bIsOwned = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item")
	FStandardUIData UIData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Store Item", meta = (Categories = "Item.Category"))
	FGameplayTagContainer Categories;

	bool operator==(const FStoreItem& Other) const
	{
		return ItemId == Other.ItemId
			&& Cost == Other.Cost
			&& bIsOwned == Other.bIsOwned
			&& UIData == Other.UIData
			&& Categories == Other.Categories;
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
enum class EStatefulInteraction : uint8
{
	None,
	Hovered,
	Unhovered,
	Clicked,
};

/** Wrapper - Generic stateful channel for communicating simple widget interactions through the VM to any listeners. */
USTRUCT(BlueprintType)
struct FInteractionState
{
	GENERATED_BODY()

	explicit FInteractionState(const EStatefulInteraction InType = EStatefulInteraction::None, const FGameplayTag InSource = FGameplayTag::EmptyTag)
		: Type(InType), Source(InSource) {}

	/** The latest interaction that occurred. */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	EStatefulInteraction Type = EStatefulInteraction::None;

	/*
	 * The source of the interaction (e.g., "InteractionSource.TabList", "InteractionSource.DetailsPanel").
	 * This is a tool to solve a problem in MVVM where the same ViewModel is used in multiple contexts and the source of the interaction is needed to differentiate them.
	 * An empty tag means the interaction is not from a specific source, or the source is not relevant.
	 */
	UPROPERTY(BlueprintReadWrite, DisplayName = "Source", Category = "Interaction", Meta = (Categories = "MolecularUI.InteractionSource"))
	FGameplayTag Source = FGameplayTag::EmptyTag;

	bool operator==(const FInteractionState& Other) const
	{
		return Type == Other.Type && Source == Other.Source;
	}

	bool IsValid() const
	{
		return Type != EStatefulInteraction::None;
	}

	FString ToString() const
	{
		FString TypeString;
		switch (Type)
		{
			case EStatefulInteraction::None: TypeString = TEXT("None"); break;
			case EStatefulInteraction::Hovered: TypeString = TEXT("Hovered"); break;
			case EStatefulInteraction::Unhovered: TypeString = TEXT("Unhovered"); break;
			case EStatefulInteraction::Clicked: TypeString = TEXT("Clicked"); break;
			default: TypeString = TEXT("Unknown"); break;
		}
		return FString::Printf(TEXT("InteractionState: Type=%s, Source=%s"), *TypeString, *Source.ToString());
	}
};

USTRUCT(BlueprintType)
struct FCategoryTabDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category Tab", Meta = (Categories = "Item.Category"))
	FGameplayTag CategoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category Tab")
	FStandardUIData UIData;
};

// Used to define how a user can select items in the UI, managed by ViewModel
UENUM(BlueprintType)
enum class EMolecularSelectionMode : uint8
{
	None,
	Single,
	SingleToggle,
	Multi,
	MultiLimited
};