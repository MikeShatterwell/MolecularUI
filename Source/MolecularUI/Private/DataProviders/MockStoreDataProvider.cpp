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
	auto SuccessWrapper = [this, OnSuccess]()
	{
		if (BackendStoreItems.IsEmpty())
		{
			CreateDummyStoreData();
		}
		OnSuccess(BackendStoreItems, FText::FromString(TEXT("Store items loaded.")));
	};

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
	auto SuccessWrapper = [this, OnSuccess]()
	{
		if (BackendOwnedStoreItems.IsEmpty())
		{
			CreateDummyOwnedStoreData();
		}
		OnSuccess(BackendOwnedStoreItems, FText::FromString(TEXT("Owned items loaded.")));
	};

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
	auto SuccessWrapper = [this, OnSuccess]()
	{
		if (BackendPlayerCurrency < 0)
		{
			CreateDummyPlayerCurrency();
		}
		OnSuccess(BackendPlayerCurrency, FText::FromString(TEXT("Currency loaded.")));
	};

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
	auto SuccessWrapper = [this, Request, OnSuccess, OnFailure]()
	{
		FStoreItem* FoundItem = BackendStoreItems.FindByPredicate([&](const FStoreItem& Item)
		{
			return Item.ItemId == Request.ItemId;
		});
		if (!FoundItem)
		{
			OnFailure(FText::FromString(TEXT("Item not found.")));
			return;
		}
		const bool bCanAfford = BackendPlayerCurrency >= FoundItem->Cost;
		if (!bCanAfford || FoundItem->bIsOwned)
		{
			OnFailure(FText::FromString(TEXT("Insufficient currency or item owned.")));
			return;
		}
		FoundItem->bIsOwned = true;
		
		FStoreItem PurchasedItem = *FoundItem;
		BackendPlayerCurrency -= PurchasedItem.Cost;
		BackendOwnedStoreItems.AddUnique(PurchasedItem); // Add a copy to owned items.
		OnSuccess(FText::FromString(TEXT("Purchase successful.")));
	};

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
	auto SuccessWrapper = [this, Request, OnSuccess, OnFailure]()
	{
		FStoreItem* FoundItem = BackendOwnedStoreItems.FindByPredicate([&](const FStoreItem& Item)
		{
			return Item.ItemId == Request.ItemId;
		});
		if (!FoundItem)
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
		
		// Remove from owned items list
		BackendOwnedStoreItems.RemoveAll([&](const FStoreItem& Item){ return Item.ItemId == Request.ItemId; });

		// Refund half cost
		BackendPlayerCurrency += FoundItem->Cost / 2;
		
		OnSuccess(FText::FromString(TEXT("Sale successful.")));
	};

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
	BackendStoreItems.Empty(10);
	BackendStoreItems.Add(FStoreItem{FName{"HealthPotion"}, 50, false, FItemUIData{INVTEXT("Health Potion")}});
	BackendStoreItems.Add(FStoreItem{FName{"Sword"}, 100, false, FItemUIData{INVTEXT("Sword")}});
	BackendStoreItems.Add(FStoreItem{FName{"Shield"}, 75, false, FItemUIData{INVTEXT("Shield")}});
}

void UMockStoreDataProvider::CreateDummyOwnedStoreData()
{
	BackendOwnedStoreItems.Empty(3);
	BackendOwnedStoreItems.Add(FStoreItem{FName{"LameSword"}, 5, true, FItemUIData{INVTEXT("Lame Sword")}});
}

void UMockStoreDataProvider::CreateDummyPlayerCurrency()
{
	BackendPlayerCurrency = 500;
}

UWorld* UMockStoreDataProvider::GetWorld() const
{
	return OuterWorld.Get();
}

