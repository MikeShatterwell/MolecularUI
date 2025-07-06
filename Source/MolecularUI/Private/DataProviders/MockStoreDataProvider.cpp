// Copyright Mike Desrosiers, All Rights Reserved.

#include "DataProviders/MockStoreDataProvider.h"
#include "Utils/MolecularMacros.h"
#include "Utils/MolecularCVars.h"
#include "MolecularTypes.h"

#include <TimerManager.h>
#include <Engine/World.h>

#include "MolecularUISettings.h"
#include "MolecularUITags.h"
#include "Utils/LogMolecularUI.h"

void UMockStoreDataProvider::InitializeProvider(UObject* InOuter)
{
	OuterWorld = InOuter ? InOuter->GetWorld() : nullptr;
}

void UMockStoreDataProvider::FetchStoreItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
											 TFunction<void(const FText&)> OnFailure)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	/*Callback*/
	auto SuccessWrapper = [this, OnSuccess]()
	{
		auto OnDataCreated = [this, OnSuccess]()
		{
			OnSuccess(BackendStoreItems, FText::FromString(TEXT("Store items loaded.")));
		};

		if (!bDummyStoreDataInitialized)
		{
			CreateDummyStoreData(OnDataCreated);
		}
		else
		{
			OnDataCreated();
		}
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
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	/*Callback*/
	auto SuccessWrapper = [this, OnSuccess]()
	{
		auto OnDataCreated = [this, OnSuccess]()
		{
			OnSuccess(BackendOwnedStoreItems, FText::FromString(TEXT("Owned items loaded.")));
		};

		if (!bDummyOwnedDataInitialized)
		{
			CreateDummyOwnedStoreData(OnDataCreated);
		}
		else
		{
			OnDataCreated();
		}
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
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

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
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

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
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

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

void UMockStoreDataProvider::LoadItemsFromDataTable(
	const TSoftObjectPtr<UDataTable>& DataTable,
	TArray<FStoreItem>& TargetArray,
	TFunction<void()> OnComplete) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	if (!DataTable.ToSoftObjectPath().IsValid())
	{
		UE_LOG(LogMolecularUI, Error, TEXT("[%hs] Data table is not valid!"), __FUNCTION__);
		return;
	}

	const FLoadSoftObjectPathAsyncDelegate LoadDelegate = FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this, &TargetArray, OnComplete](const FSoftObjectPath& SoftPath, UObject* LoadedObject)
		{
			const UDataTable* LoadedTable = Cast<UDataTable>(LoadedObject);
			if (!IsValid(LoadedTable))
			{
				UE_LOG(LogMolecularUI, Error, TEXT("[%hs] Failed to load data table."), __FUNCTION__);
				if (OnComplete)
				{
					OnComplete();
				}
				return;
			}

			TArray<FStoreItem*> Items;
			LoadedTable->GetAllRows<FStoreItem>(__FUNCTION__, Items);
			for (const FStoreItem* Item : Items)
			{
				if (ensure(Item != nullptr) && !Item->ItemId.IsNone())
				{
					TargetArray.Add(*Item);
				}
			}
			if (OnComplete)
			{
				OnComplete();
			}
		});

	(void)DataTable.LoadAsync(LoadDelegate);
}

void UMockStoreDataProvider::CreateDummyStoreData(TFunction<void()> OnComplete)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	BackendStoreItems.Empty();

	auto OnDataTableLoaded = [this, OnComplete]()
	{
		const int32 NumItems = FMath::Clamp(
		MolecularUI::CVars::Store::NumDummyItems,
		1,
		10000);

		for (int32 Index = 0; Index < NumItems; ++Index)
		{
			const FString DisplayName = FString::Printf(TEXT("Mock Store Item %s"), *FString::FromInt(Index + 1));
			const FString ItemIdString = FString::Printf(TEXT("Id: %d"), Index + 1);
			const int32 Cost = 10 + Index * 5;

			FSlateBrush IconBrush = UMolecularUISettings::GetDefaultStoreIcon();
			IconBrush.TintColor = FLinearColor::MakeRandomColor(); // Random color for the icon

			BackendStoreItems.Add(FStoreItem{
				FName{*ItemIdString},
				Cost,
				false,
				FItemUIData{FText::FromString(DisplayName), 
							FText::FromString(FString::Printf(TEXT("Dummy description for %s (Id: %s)"), *DisplayName, *ItemIdString)),
							IconBrush,
				FGameplayTagContainer(MolecularUITags::Item::Category::Other)}
			});
		}

		bDummyStoreDataInitialized = true;
		if (OnComplete)
		{
			OnComplete();
		}
	};

	LoadItemsFromDataTable(
		UMolecularUISettings::GetDefaultStoreItemsDataTable(),
		BackendStoreItems,
		OnDataTableLoaded);
}

void UMockStoreDataProvider::CreateDummyOwnedStoreData(TFunction<void()> OnComplete)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	BackendOwnedStoreItems.Empty();

	auto OnDataTableLoaded = [this, OnComplete]()
	{
		bDummyOwnedDataInitialized = true;
		if (OnComplete)
		{
			OnComplete();
		}
	};

	LoadItemsFromDataTable(
		UMolecularUISettings::GetDefaultOwnedItemsDataTable(),
		BackendOwnedStoreItems,
		OnDataTableLoaded);
}

void UMockStoreDataProvider::CreateDummyPlayerCurrency()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	BackendPlayerCurrency = 500;
	bDummyPlayerCurrencyInitialized = true;
}

UWorld* UMockStoreDataProvider::GetWorld() const
{
	return OuterWorld.Get();
}
