// Copyright Mike Desrosiers, All Rights Reserved.

#include "Subsystems/StoreSubsystem.h"

#include <TimerManager.h>

#include "ViewModels/StoreViewModel.h"
#include "ViewModels/ItemViewModel.h"
#include "Utils/MolecularMacros.h"
#include "MolecularUITags.h"
#include "Utils/LogMolecularUI.h"
#include "DataProviders/MockStoreDataProvider.h"

namespace
{
	/** RAII helper that tracks a store state for the lifetime of an async operation. */
	struct FScopedStoreState
	{
		TWeakObjectPtr<UStoreViewModel> ViewModel;
		FGameplayTag State;

		FScopedStoreState(UStoreViewModel* InViewModel, const FGameplayTag& InState)
			: ViewModel(InViewModel), State(InState)
		{
			if (ViewModel.IsValid())
			{
				ViewModel->AddStoreState(State);
			}
		}

		~FScopedStoreState()
		{
			if (ViewModel.IsValid())
			{
				ViewModel->RemoveStoreState(State);
			}
		}
	};
}

void UStoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

		StoreViewModel = NewObject<UStoreViewModel>(this);

		// Spawn the provider from the configurable class. By default this will
		// create the mock implementation but it can be replaced with any class
		// that implements IStoreDataProvider.
		if (DataProviderClass)
		{
			DataProviderObject = NewObject<UObject>(this, DataProviderClass);
		}

		if (DataProviderObject && DataProviderObject->GetClass()->ImplementsInterface(UStoreDataProvider::StaticClass()))
		{
			DataProvider.SetObject(DataProviderObject);
			DataProvider.SetInterface(Cast<IStoreDataProvider>(DataProviderObject));

			// Initialize mock provider with our world for timer usage.
			if (UMockStoreDataProvider* MockProvider = Cast<UMockStoreDataProvider>(DataProviderObject))
			{
				MockProvider->InitializeProvider(this);
			}
		}

	// Start in a "None" state so initial loads can be triggered on first access.
	StoreViewModel->AddStoreState(MolecularUITags::Store::State_None);

	UE_MVVM_BIND_FIELD(StoreViewModel, FilterText, OnFilterTextChanged);
	UE_MVVM_BIND_FIELD(StoreViewModel, TransactionRequest, OnTransactionRequestChanged);
	UE_MVVM_BIND_FIELD(StoreViewModel, bRefreshRequested, OnRefreshRequestedChanged);
}

void UStoreSubsystem::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	UE_MVVM_UNBIND_FIELD(StoreViewModel, FilterText);
	UE_MVVM_UNBIND_FIELD(StoreViewModel, TransactionRequest);

	StoreViewModel = nullptr;
	for (const auto& Pair : ItemViewModelCache)
	{
		if (IsValid(Pair.Value))
		{
			// Unbind any field notifications from the ItemViewModel.
			UE_MVVM_UNBIND_FIELD(Pair.Value, Interaction);
		}
	}
	
       ItemViewModelCache.Empty();
       CachedStoreItems.Empty();
	
	DataProvider = nullptr;
	DataProviderObject = nullptr;

	Super::Deinitialize();
}

UStoreViewModel* UStoreSubsystem::GetStoreViewModel_Implementation()
{
	if (!IsValid(StoreViewModel))
	{
		UE_LOG(LogMolecularUI, Error, TEXT("[%hs] UStoreSubsystem is not initialized yet!"), __FUNCTION__);
		return nullptr;
	}

	if (StoreViewModel->HasStoreState(MolecularUITags::Store::State_None))
	{
		StoreViewModel->RemoveStoreState(MolecularUITags::Store::State_None);

		// Load stuff on initial open
		LazyLoadStoreItems();
		LazyLoadStoreCurrency();
		LazyLoadOwnedItems();
	}

	return StoreViewModel;
}

void UStoreSubsystem::OnFilterTextChanged(UObject* Object, UE::FieldNotification::FFieldId Field)
{
	FilterAvailableStoreItems(StoreViewModel->GetFilterText());
}

void UStoreSubsystem::OnTransactionRequestChanged(UObject* Object, UE::FieldNotification::FFieldId Field)
{
	const FTransactionRequest& TransactionRequest = StoreViewModel->GetTransactionRequest();

	// Only process valid requests.
	// When the request is processed and cleared, this will be called again with an empty request,
	// so we can safely ignore that case.
	if (!TransactionRequest.IsValid())
	{
		return;
	}

	// If the store isn't ready, queue or reset the request and exit early.
	if (!StoreViewModel->HasStoreState(MolecularUITags::Store::State_Ready))
	{
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Store not ready. Failing transaction request for %s"), __FUNCTION__,
			   *TransactionRequest.ToString());

		StoreViewModel->SetTransactionRequest(FTransactionRequest());
		StoreViewModel->SetErrorMessage(FText::FromString("Store not ready. Please try again later."));
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
		return;
	}

	switch (StoreViewModel->GetTransactionType())
	{
	case ETransactionType::None:
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] No transaction type set for request: %s"), __FUNCTION__,
			   *TransactionRequest.ToString());
		break;
	case ETransactionType::Purchase:
		LazyPurchaseItem(TransactionRequest);
		break;
	case ETransactionType::Sell:
		ensure(false); // Not implemented yet
		break;
	}
}

void UStoreSubsystem::OnRefreshRequestedChanged(UObject* Object, UE::FieldNotification::FFieldId Field)
{
	if (StoreViewModel->GetRefreshRequested())
	{
		StoreViewModel->SetRefreshRequested(false); // Reset the flag

		// Clear any existing error message
		StoreViewModel->SetErrorMessage(FText::GetEmpty());
		StoreViewModel->RemoveStoreState(MolecularUITags::Store::State_Error);

		// Refresh the store data
		LazyLoadStoreItems();
		LazyLoadOwnedItems();
		LazyLoadStoreCurrency();
		
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Refresh requested. Reloading store data."), __FUNCTION__);
	}
}

void UStoreSubsystem::OnItemInteractionChanged(UObject* Object, UE::FieldNotification::FFieldId Field)
{
	UItemViewModel* ItemVM = Cast<UItemViewModel>(Object);
	if (!ensure(IsValid(ItemVM)))
	{
		return;
	}

	const FItemInteraction& Interaction = ItemVM->GetInteraction();
	if (!Interaction.IsValid())
	{
		return; // No valid interaction to process
	}

	// Handle the interaction based on its type
	switch (Interaction.Type)
	{
	case EItemInteractionType::Hovered:
		if (GEngine)
		{
			const FString& ItemName = ItemVM->GetItemData().UIData.DisplayName.ToString();
			GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Yellow,
											 FString::Printf(TEXT("Item Hovered: %s"), *ItemName));
		}
		break;
	case EItemInteractionType::Unhovered:
		if (GEngine)
		{
			const FString& ItemName = ItemVM->GetItemData().UIData.DisplayName.ToString();
			GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Yellow,
											 FString::Printf(TEXT("Item UnHovered: %s"), *ItemName));
		}
		break;
	case EItemInteractionType::Clicked:
		if (GEngine)
		{
			const FString& ItemName = ItemVM->GetItemData().UIData.DisplayName.ToString();
			GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Yellow,
											 FString::Printf(TEXT("Item Clicked: %s"), *ItemName));
			StoreViewModel->SetSelectedItem(ItemVM);
			if (StoreViewModel->GetOwnedItems().Contains(ItemVM))
			{
				StoreViewModel->SetTransactionType(ETransactionType::Sell);
			}
			else if (StoreViewModel->GetAvailableItems().Contains(ItemVM))
			{
				StoreViewModel->SetTransactionType(ETransactionType::Purchase);
			}
			else
			{
				StoreViewModel->SetTransactionType(ETransactionType::None);
			}
		}
		break;
	default:
		break;
	}
}

void UStoreSubsystem::LazyLoadStoreItems()
{
	TSharedRef<FScopedStoreState> LoadingScope = MakeShared<FScopedStoreState>(
	StoreViewModel, MolecularUITags::Store::State_Loading_Items);

       auto OnSuccess = [this, LoadingScope](const TArray<FStoreItem>& Items, const FText& Status)
       {
               CachedStoreItems = Items;
               TArray<TObjectPtr<UItemViewModel>> StoreItems;
               StoreItems.Reserve(Items.Num());

		for (const FStoreItem& ItemData : Items)
		{
			UItemViewModel* ItemVM = GetOrCreateItemViewModel(ItemData);
			StoreItems.AddUnique(ItemVM);
		}

		StoreViewModel->SetAvailableItems(StoreItems);
		FilterAvailableStoreItems(StoreViewModel->GetFilterText());
		StoreViewModel->SetStatusMessage(Status);
	};

	auto OnFailure = [this, LoadingScope](const FText& Error)
	{
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Failure loading store items."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(Error);
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
	};

	if (DataProvider)
	{
		DataProvider->FetchStoreItems(OnSuccess, OnFailure);
	}
}

void UStoreSubsystem::LazyLoadOwnedItems()
{
	TSharedRef<FScopedStoreState> LoadingScope = MakeShared<FScopedStoreState>(
	StoreViewModel, MolecularUITags::Store::State_Loading_OwnedItems);

	auto OnSuccess = [this, LoadingScope](const TArray<FStoreItem>& Items, const FText& Status)
	{
		TArray<TObjectPtr<UItemViewModel>> OwnedItemVMs;
		OwnedItemVMs.Reserve(Items.Num());

		for (const FStoreItem& OwnedItemData : Items)
		{
				UItemViewModel* ItemVM = GetOrCreateItemViewModel(OwnedItemData);
				OwnedItemVMs.AddUnique(ItemVM);
		}

		StoreViewModel->SetOwnedItems(OwnedItemVMs);
		StoreViewModel->SetStatusMessage(Status);
	};

	auto OnFailure = [this, LoadingScope](const FText& Error)
	{
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Failure loading owned items."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(Error);
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
	};

	if (DataProvider)
	{
		DataProvider->FetchOwnedItems(OnSuccess, OnFailure);
	}
}

void UStoreSubsystem::LazyLoadStoreCurrency()
{
	TSharedRef<FScopedStoreState> LoadingScope = MakeShared<FScopedStoreState>(
	StoreViewModel, MolecularUITags::Store::State_Loading_Currency);

	auto OnSuccess = [this, LoadingScope](int32 Currency, const FText& Status)
	{
		StoreViewModel->SetPlayerCurrency(Currency);
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Player currency loaded: %d"), __FUNCTION__, Currency);
		StoreViewModel->SetStatusMessage(Status);
	};

	auto OnFailure = [this, LoadingScope](const FText& Error)
	{
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Failure loading store currency."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(Error);
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
	};

	if (DataProvider)
	{
		DataProvider->FetchPlayerCurrency(OnSuccess, OnFailure);
	}
}

void UStoreSubsystem::LazyPurchaseItem(const FTransactionRequest& PurchaseRequest)
{
	TSharedRef<FScopedStoreState> PurchaseScope = MakeShared<FScopedStoreState>(
	StoreViewModel, MolecularUITags::Store::State_Purchasing);

	auto OnSuccess = [this, PurchaseScope](const FText& Status)
	{
		StoreViewModel->SetTransactionRequest(FTransactionRequest());
		StoreViewModel->SetTransactionType(ETransactionType::None);
		StoreViewModel->SetSelectedItem(nullptr);

		LazyLoadStoreItems();
		LazyLoadOwnedItems();
		StoreViewModel->SetStatusMessage(Status);
	};

	auto OnFailure = [this, PurchaseScope](const FText& Error)
	{
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
		StoreViewModel->SetErrorMessage(Error);
	};

	if (DataProvider)
	{
		DataProvider->PurchaseItem(PurchaseRequest, OnSuccess, OnFailure);
	}
}

void UStoreSubsystem::FilterAvailableStoreItems(const FString& FilterText)
{
       if (CachedStoreItems.IsEmpty())
       {
               return;
       }
       TArray<TObjectPtr<UItemViewModel>> FilteredItems;
       FilteredItems.Reserve(CachedStoreItems.Num());

       for (const FStoreItem& ItemData : CachedStoreItems)
       {
               if (FilterText.IsEmpty() || ItemData.UIData.DisplayName.ToString().Contains(FilterText))
               {
                       UItemViewModel* ItemVM = GetOrCreateItemViewModel(ItemData);
                       FilteredItems.Add(ItemVM);
               }
       }

       StoreViewModel->SetAvailableItems(FilteredItems);
}


UItemViewModel* UStoreSubsystem::GetOrCreateItemViewModel(const FStoreItem& ItemData)
{
	// Check if the ViewModel already exists in the cache and is valid.
	if (TObjectPtr<UItemViewModel>* FoundViewModel = ItemViewModelCache.Find(ItemData.ItemId))
	{
		if (IsValid(*FoundViewModel))
		{
			// It exists, update its data just in case it changed and return it.
			(*FoundViewModel)->SetItemData(ItemData);
			return *FoundViewModel;
		}
	}

	// If not found or was invalid (e.g. garbage collected), create a new one.
	UItemViewModel* NewItemVM = NewObject<UItemViewModel>(StoreViewModel);
	NewItemVM->SetItemData(ItemData);

	// Bind the interaction FieldNotify to the ViewModel's OnItemInteractionChanged handler.
	UE_MVVM_BIND_FIELD(NewItemVM, Interaction, OnItemInteractionChanged);

	// Add the new ViewModel to the cache for future reuse.
	ItemViewModelCache.Add(ItemData.ItemId, NewItemVM);

	return NewItemVM;
}

void UStoreSubsystem::BreakErrorState()
{
	if (IsValid(StoreViewModel) && StoreViewModel->HasStoreState(MolecularUITags::Store::State_Error))
	{
		StoreViewModel->SetErrorMessage(FText::GetEmpty());
		StoreViewModel->RemoveStoreState(MolecularUITags::Store::State_Error);
	}
}
