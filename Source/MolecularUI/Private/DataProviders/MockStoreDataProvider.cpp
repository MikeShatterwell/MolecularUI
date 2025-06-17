#include "DataProviders/StoreDataProvider.h"
#include "Utils/MolecularMacros.h"
#include "ViewModels/ItemViewModel.h"
#include "MolecularTypes.h"
#include "MolecularUITags.h"
#include <TimerManager.h>

#include "Engine/World.h"

#include "MockStoreDataProvider.h"

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

    FETCH_MOCK_DATA(ItemLoadHandle, SuccessWrapper, FailureWrapper, 0.15f, 3.5f, 8.5f);
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

    FETCH_MOCK_DATA(OwnedItemLoadHandle, SuccessWrapper, FailureWrapper);
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

    FETCH_MOCK_DATA(CurrencyLoadHandle, SuccessWrapper, FailureWrapper);
}

void UMockStoreDataProvider::PurchaseItem(const FTransactionRequest& Request,
                                          TFunction<void(const FText&)> OnSuccess,
                                          TFunction<void(const FText&)> OnFailure)
{
    auto SuccessWrapper = [this, Request, OnSuccess]()
    {
        const FStoreItem* FoundItem = BackendStoreItems.FindByPredicate([&](const FStoreItem& Item)
        {
            return Item.ItemId == Request.ItemId;
        });
        if (!FoundItem)
        {
            OnFailure(FText::FromString(TEXT("Item not found.")));
            return;
        }
        if (BackendPlayerCurrency < FoundItem->Cost || BackendOwnedStoreItems.Contains(*FoundItem))
        {
            OnFailure(FText::FromString(TEXT("Insufficient currency or item owned.")));
            return;
        }
        BackendPlayerCurrency -= FoundItem->Cost;
        BackendOwnedStoreItems.Add(*FoundItem);
        BackendStoreItems.Remove(*FoundItem);
        OnSuccess(FText::FromString(TEXT("Purchase successful.")));
    };

    auto FailureWrapper = [OnFailure]()
    {
        OnFailure(FText::FromString(TEXT("Purchase failed.")));
    };

    FETCH_MOCK_DATA(PurchaseHandle, SuccessWrapper, FailureWrapper);
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

