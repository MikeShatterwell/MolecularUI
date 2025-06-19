// Copyright Mike Desrosiers, All Rights Reserved.

#include "DataProviders/MockStoreDataProvider.h"
#include "Utils/MolecularMacros.h"
#include "Utils/MolecularCVars.h"
#include "MolecularTypes.h"

#include <TimerManager.h>
#include <Engine/World.h>

void UMockStoreDataProvider::InitializeProvider(UObject* InOuter)
{
	OuterWorld = InOuter ? InOuter->GetWorld() : nullptr;
}

void UMockStoreDataProvider::FetchStoreItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
											 TFunction<void(const FText&)> OnFailure)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	/*Callback*/
	auto SuccessWrapper = [this, OnSuccess]()
	{
		if (!bDummyStoreDataInitialized)
		{
			CreateDummyStoreData();
		}
		OnSuccess(BackendStoreItems, FText::FromString(TEXT("Store items loaded.")));
	};

	/*Callback*/
	auto FailureWrapper = [OnFailure]()
	{
		OnFailure(FText::FromString(TEXT("Failed to load store items.")));
	};

	FETCH_MOCK_DATA(ItemLoadHandle, SuccessWrapper, FailureWrapper,
					MolecularUI::CVars::Store::FailureChance,
					MolecularUI::CVars::Store::MinDelay,
					MolecularUI::CVars::Store::MaxDelay);
}

void UMockStoreDataProvider::FetchOwnedItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
											 TFunction<void(const FText&)> OnFailure)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	/*Callback*/
	auto SuccessWrapper = [this, OnSuccess]()
	{
		if (!bDummyOwnedDataInitialized)
		{
			CreateDummyOwnedStoreData();
		}
		OnSuccess(BackendOwnedStoreItems, FText::FromString(TEXT("Owned items loaded.")));
	};

	/*Callback*/
	auto FailureWrapper = [OnFailure]()
	{
		OnFailure(FText::FromString(TEXT("Failed to load owned items.")));
	};

	FETCH_MOCK_DATA(OwnedItemLoadHandle, SuccessWrapper, FailureWrapper,
					MolecularUI::CVars::OwnedItems::FailureChance,
					MolecularUI::CVars::OwnedItems::MinDelay,
					MolecularUI::CVars::OwnedItems::MaxDelay);
}

void UMockStoreDataProvider::FetchPlayerCurrency(TFunction<void(int32, const FText&)> OnSuccess,
												 TFunction<void(const FText&)> OnFailure)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	/*Callback*/
	auto SuccessWrapper = [this, OnSuccess]()
	{
		if (!bDummyPlayerCurrencyInitialized)
		{
			CreateDummyPlayerCurrency();
		}
		OnSuccess(BackendPlayerCurrency, FText::FromString(TEXT("Currency loaded.")));
	};

	/*Callback*/
	auto FailureWrapper = [OnFailure]()
	{
		OnFailure(FText::FromString(TEXT("Failed to load currency.")));
	};

	FETCH_MOCK_DATA(CurrencyLoadHandle, SuccessWrapper, FailureWrapper,
					MolecularUI::CVars::PlayerCurrency::FailureChance,
					MolecularUI::CVars::PlayerCurrency::MinDelay,
					MolecularUI::CVars::PlayerCurrency::MaxDelay);
}

void UMockStoreDataProvider::PurchaseItem(const FTransactionRequest& Request,
										  TFunction<void(const FText&)> OnSuccess,
										  TFunction<void(const FText&)> OnFailure)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	/*Callback*/
	auto SuccessWrapper = [this, Request, OnSuccess, OnFailure]()
	{
		FStoreItem* FoundItem = BackendStoreItems.FindByPredicate([&](const FStoreItem& Item)
		{
			return Item.ItemId == Request.ItemId;
		});
		if (FoundItem == nullptr)
		{
			OnFailure(FText::FromString(TEXT("Item not found.")));
			return;
		}
		const bool bCanPurchase = BackendPlayerCurrency >= FoundItem->Cost;
		if (!bCanPurchase || FoundItem->bIsOwned)
		{
			OnFailure(FText::FromString(TEXT("Insufficient currency or item owned.")));
			return;
		}

		FoundItem->bIsOwned = true; // Mark the item as owned in the store items list.

		FStoreItem PurchasedItem = *FoundItem;
		BackendPlayerCurrency -= PurchasedItem.Cost; // Deduct full cost from player currency.
		BackendOwnedStoreItems.AddUnique(PurchasedItem); // Add a copy to owned items.
		BackendStoreItems.RemoveAll([&](const FStoreItem& Item) { return Item.ItemId == Request.ItemId; });
		// Remove from available items.
		OnSuccess(FText::FromString(TEXT("Purchase successful.")));
	};

	/*Callback*/
	auto FailureWrapper = [OnFailure]()
	{
		OnFailure(FText::FromString(TEXT("Purchase failed.")));
	};

	FETCH_MOCK_DATA(TransactionHandle, SuccessWrapper, FailureWrapper,
					MolecularUI::CVars::Transaction::FailureChance,
					MolecularUI::CVars::Transaction::MinDelay,
					MolecularUI::CVars::Transaction::MaxDelay);
}

void UMockStoreDataProvider::SellItem(const FTransactionRequest& Request, TFunction<void(const FText&)> OnSuccess,
									  TFunction<void(const FText&)> OnFailure)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	/*Callback*/
	auto SuccessWrapper = [this, Request, OnSuccess, OnFailure]()
	{
		FStoreItem* FoundItem = BackendOwnedStoreItems.FindByPredicate([&](const FStoreItem& Item)
		{
			return Item.ItemId == Request.ItemId;
		});
		if (FoundItem == nullptr)
		{
			OnFailure(FText::FromString(TEXT("Owned item not found.")));
			return;
		}

		const bool bCanSell = FoundItem->bIsOwned;
		if (!bCanSell)
		{
			OnFailure(FText::FromString(TEXT("Item not owned.")));
			return;
		}

		FoundItem->bIsOwned = false;

		FStoreItem SoldItem = *FoundItem; // Copy the item to sell
		BackendPlayerCurrency += SoldItem.Cost / 2; // Refund half the cost to player currency.
		BackendOwnedStoreItems.RemoveAll([&](const FStoreItem& Item) { return Item.ItemId == Request.ItemId; });
		// Remove from owned items.
		BackendStoreItems.AddUnique(SoldItem); // Add back to available items
		OnSuccess(FText::FromString(TEXT("Sale successful.")));
	};

	/*Callback*/
	auto FailureWrapper = [OnFailure]()
	{
		OnFailure(FText::FromString(TEXT("Sale failed.")));
	};

	FETCH_MOCK_DATA(TransactionHandle, SuccessWrapper, FailureWrapper,
					MolecularUI::CVars::Transaction::FailureChance,
					MolecularUI::CVars::Transaction::MinDelay,
					MolecularUI::CVars::Transaction::MaxDelay);
}

void UMockStoreDataProvider::CreateDummyStoreData()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	const int32 NumItems = FMath::Clamp(
		MolecularUI::CVars::Store::NumDummyItems,
		1,
		1000);

	const TArray<FString> Adjectives = {
		TEXT("Ancient"),
		TEXT("Mystic"),
		TEXT("Enchanted"),
		TEXT("Crimson"),
		TEXT("Golden"),
		TEXT("Arcane"),
		TEXT("Shadow"),
		TEXT("Emerald"),
		TEXT("Basic"),
		TEXT("Rusty"),
		TEXT("Cursed"),
		TEXT("Divine"),
		TEXT("Frost"),
		TEXT("Fiery"),
		TEXT("Silver"),
		TEXT("Iron"),
		TEXT("Steel"),
		TEXT("Obsidian"),
		TEXT("Celestial"),
		TEXT("Phantom"),
		TEXT("Legendary"),
		TEXT("Heroic"),
		TEXT("Epic"),
		TEXT("Mythic"),
		TEXT("Titanic"),
		TEXT("Vengeful"),
		TEXT("Radiant"),
		TEXT("Spectral"),
		TEXT("Wicked"),
		TEXT("Valiant"),
		TEXT("Brave"),
		TEXT("Noble"),
		TEXT("Swift"),
	};

	const TArray<FString> Nouns = {
		TEXT("Sword"),
		TEXT("Shield"),
		TEXT("Potion"),
		TEXT("Amulet"),
		TEXT("Bow"),
		TEXT("Helm"),
		TEXT("Dagger"),
		TEXT("Staff"),
		TEXT("Ring"),
		TEXT("Boots"),
		TEXT("Gauntlets"),
		TEXT("Cloak"),
		TEXT("Axe"),
		TEXT("Spear"),
		TEXT("Wand"),
		TEXT("Tome"),
		TEXT("Scroll"),
		TEXT("Belt"),
		TEXT("Gem"),
		TEXT("Crystal"),
		TEXT("Lantern"),
		TEXT("Mask"),
		TEXT("Cape"),
		TEXT("Quiver")
	};

	BackendStoreItems.Empty(NumItems);
	for (int32 Index = 0; Index < NumItems; ++Index)
	{
		const FString Adjective = Adjectives[Index % Adjectives.Num()];
		const FString Noun = Nouns[Index % Nouns.Num()];

		const FString DisplayName = FString::Printf(TEXT("%s %s"), *Adjective, *Noun);
		const FString ItemIdString = FString::Printf(TEXT("%s_%s_%d"), *Adjective, *Noun, Index + 1);
		const int32 Cost = 10 + Index * 5;

		BackendStoreItems.Add(FStoreItem{
			FName{*ItemIdString},
			Cost,
			false,
			FItemUIData{FText::FromString(DisplayName)}
		});
	}

	bDummyStoreDataInitialized = true;
}

void UMockStoreDataProvider::CreateDummyOwnedStoreData()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	BackendOwnedStoreItems.Empty(1);
	const FString DisplayName = TEXT("Rusty Sword");
	const FString ItemIdString = TEXT("Rusty_Sword");
	BackendOwnedStoreItems.Add(FStoreItem{
		FName{*ItemIdString},
		10,
		true,
		FItemUIData{FText::FromString(DisplayName)}
	});

	bDummyOwnedDataInitialized = true;
}

void UMockStoreDataProvider::CreateDummyPlayerCurrency()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	BackendPlayerCurrency = 500;
	bDummyPlayerCurrencyInitialized = true;
}

UWorld* UMockStoreDataProvider::GetWorld() const
{
	return OuterWorld.Get();
}
