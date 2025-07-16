// Copyright Mike Desrosiers, All Rights Reserved.

#include "DataProviders/MockStoreDataProviderSubsystem.h"
#include "Utils/MolecularMacros.h"
#include "Utils/MolecularCVars.h"
#include "MolecularTypes.h"

#include <TimerManager.h>
#include <Engine/World.h>

#include "MolecularUISettings.h"
#include "MolecularUITags.h"
#include "Utils/LogMolecularUI.h"

void UMockStoreDataProviderSubsystem::FetchStoreItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
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

void UMockStoreDataProviderSubsystem::FetchOwnedItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
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

void UMockStoreDataProviderSubsystem::FetchPlayerCurrency(TFunction<void(int32, const FText&)> OnSuccess,
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

void UMockStoreDataProviderSubsystem::PurchaseItem(const FTransactionRequest& Request,
										  TFunction<void(const FText&)> OnSuccess,
										  TFunction<void(const FText&)> OnFailure)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	/*Callback*/
	auto SuccessWrapper = [this, /*FTransactionRequest*/ Request, OnSuccess, OnFailure]()
	{
		// This represents the actual backend/server check for purchasing items.
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Backend call successful! Carrying out purchase request: %s"), __FUNCTION__, *Request.ToString());
		int32 TotalCost = 0;
		TArray<FStoreItem*> ItemsToPurchase;
		for (const FName& ItemId : Request.ItemIds)
		{
			FStoreItem* FoundItem = BackendStoreItems.FindByPredicate([&](const FStoreItem& Item)
			{
				return Item.ItemId == ItemId;
			});
			if (FoundItem == nullptr)
			{
				const FString& ErrorText = FString::Printf(TEXT("Item %s not found in store."), *ItemId.ToString());
				UE_LOG(LogMolecularUI, Error, TEXT("%s"), *ErrorText);
				OnFailure(FText::FromString(ErrorText)); // TODO: Handle localization properly on all user-facing messages.
				return;
			}
			if (FoundItem->bIsOwned)
			{
				const FString& ErrorText = FString::Printf(TEXT("[%hs] Item %s is already owned and cannot be purchased."), __FUNCTION__, *FoundItem->ItemId.ToString());
				UE_LOG(LogMolecularUI, Error, TEXT("%s"), *ErrorText);
				OnFailure(FText::FromString(ErrorText));
				return;
			}
			UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Found item %s with cost %d."), __FUNCTION__, *FoundItem->ItemId.ToString(), FoundItem->Cost);
			TotalCost += FoundItem->Cost;
			ItemsToPurchase.Add(FoundItem);
		}
		if (BackendPlayerCurrency < TotalCost)
		{
			const FString& ErrorText = FString::Printf(TEXT("[%hs] Insufficient currency to carry out the purchase: %s"), __FUNCTION__, *Request.ToString());
			UE_LOG(LogMolecularUI, Error, TEXT("%s"), *ErrorText);
			OnFailure(FText::FromString(FString::Printf(TEXT("Insufficient currency to carry out the purchase: %s"), *Request.ToString())));
			return;
		}

		// Simulate the purchase
		for (FStoreItem* Item : ItemsToPurchase)
		{
			UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Simulating purchase of item %s with cost %d."), __FUNCTION__, *Item->ItemId.ToString(), Item->Cost);
			Item->bIsOwned = true;
			BackendPlayerCurrency -= Item->Cost;
			BackendOwnedStoreItems.AddUnique(*Item);
			BackendStoreItems.RemoveAll([&](const FStoreItem& StoreItem){ return StoreItem.ItemId == Item->ItemId; });
		}

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

void UMockStoreDataProviderSubsystem::SellItem(const FTransactionRequest& Request, TFunction<void(const FText&)> OnSuccess,
									  TFunction<void(const FText&)> OnFailure)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	/*Callback*/
	auto SuccessWrapper = [this, /*FTransactionRequest*/ Request, OnSuccess, OnFailure]()
	{
		int32 Refund = 0;
		TArray<FStoreItem*> ItemsToSell;
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Backend call successful! Carrying out sale request: %s"), __FUNCTION__, *Request.ToString());
		for (const FName& ItemId : Request.ItemIds)
		{
			FStoreItem* FoundItem = BackendOwnedStoreItems.FindByPredicate([&](const FStoreItem& Item)
			{
					return Item.ItemId == ItemId;
			});
			if (FoundItem == nullptr)
			{
				const FString& ErrorText = FString::Printf(TEXT("[%hs] Item %s not found in owned items."), __FUNCTION__, *ItemId.ToString());
				UE_LOG(LogMolecularUI, Error, TEXT("%s"), *ErrorText);
				OnFailure(FText::FromString(ErrorText));
				return;
			}
			if (!FoundItem->bIsOwned)
			{
				const FString& ErrorText = FString::Printf(TEXT("[%hs] Item %s not owned and cannot be sold."), __FUNCTION__, *ItemId.ToString());
				UE_LOG(LogMolecularUI, Error, TEXT("%s"), *ErrorText);
				OnFailure(FText::FromString(ErrorText));
				return;
			}
			Refund += FoundItem->Cost / 2; // Arbitrary refund logic, e.g., 50% of the cost. TODO: Make this configurable.

			UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Found item %s with cost %d."), __FUNCTION__, *FoundItem->ItemId.ToString(), FoundItem->Cost);
			ItemsToSell.Add(FoundItem);
		}

		for (FStoreItem* Item : ItemsToSell)
		{
			UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Simulating sale of item %s with refund %d."), __FUNCTION__, *Item->ItemId.ToString(), Item->Cost / 2);
			Item->bIsOwned = false;
			BackendOwnedStoreItems.RemoveAll([&](const FStoreItem& Owned){ return Owned.ItemId == Item->ItemId; });
			BackendStoreItems.AddUnique(*Item);
		}
		BackendPlayerCurrency += Refund;
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

void UMockStoreDataProviderSubsystem::LoadItemsFromDataTable(
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

void UMockStoreDataProviderSubsystem::CreateDummyStoreData(TFunction<void()> OnComplete)
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
				FStandardUIData{
					/*InDisplayName*/ FText::FromString(DisplayName),
					/*InDescription*/ FText::FromString(FString::Printf(TEXT("Dummy description for %s (Id: %s)"), *DisplayName, *ItemIdString)),
					/*InIcon*/ IconBrush},
				FGameplayTagContainer(MolecularUITags::Item::Category::Other)}
			);
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

void UMockStoreDataProviderSubsystem::CreateDummyOwnedStoreData(TFunction<void()> OnComplete)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	BackendOwnedStoreItems.Empty(BackendStoreItems.Num());
	for (const FStoreItem& StoreItem : BackendStoreItems)
	{
		if (StoreItem.bIsOwned)
		{
			BackendOwnedStoreItems.Add(StoreItem);
		}
	}

	bDummyOwnedDataInitialized = true;
	if (OnComplete)
	{
		// No need for an async load here since we are just copying from the already loaded store items.
		OnComplete();
	}
}

void UMockStoreDataProviderSubsystem::CreateDummyPlayerCurrency()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	BackendPlayerCurrency = 500;
	bDummyPlayerCurrencyInitialized = true;
}