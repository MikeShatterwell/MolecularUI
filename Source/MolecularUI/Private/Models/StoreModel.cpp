﻿// Copyright Mike Desrosiers, All Rights Reserved.

#include "Models/StoreModel.h"

#include <MVVMGameSubsystem.h>
#include <TimerManager.h>

#include "ViewModels/StoreViewModel.h"
#include "ViewModels/ItemViewModel.h"
#include "Utils/MolecularMacros.h"
#include "MolecularUITags.h"
#include "Utils/LogMolecularUI.h"
#include "DataProviders/MockStoreDataProviderSubsystem.h"
#include "ViewModels/CategoryViewModel.h"
#include "ViewModels/SelectionViewModel.h"

namespace UStoreSubsystem_private
{
	/** RAII helper that tracks a store state for the lifetime of an async operation.
	*
	* 	TODO: Could be used to create a scoped store state in a more modular way using an interface that the ViewModel implements.
	* 	Currently relies on UStoreViewModel having the AddStoreState and RemoveStoreState methods as a demo.
	 */
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

	#define SCOPED_STORE_STATE(VarName, ViewModelPtr, StateTag) \
		TSharedRef<UStoreSubsystem_private::FScopedStoreState> VarName = MakeShared<UStoreSubsystem_private::FScopedStoreState>(ViewModelPtr, StateTag)
}

// Begin UMolecularModelBase interface.
void UStoreModel::InitializeModel_Implementation(UWorld* World)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	Super::InitializeModel_Implementation(World);

	if (!IsValid(World))
	{
		return;
	}

	// Use a different class when not using the mock data provider.
	// This example doesn't have a "real" data provider.
	UGameInstanceSubsystem* StoreDataProviderSubsystem = World->GetGameInstance()->GetSubsystem<UMockStoreDataProviderSubsystem>();

	if (IsValid(StoreDataProviderSubsystem)
		&& StoreDataProviderSubsystem->GetClass()->ImplementsInterface(UStoreDataProvider::StaticClass()))
	{
		StoreDataProviderInterface.SetObject(StoreDataProviderSubsystem);
		StoreDataProviderInterface.SetInterface(Cast<IStoreDataProvider>(StoreDataProviderSubsystem));
	}

	StoreViewModelCollection = NewObject<UMVVMViewModelCollectionObject>(this, UMVVMViewModelCollectionObject::StaticClass());

	if (!IsValid(StoreViewModel))
	{
		StoreViewModel = NewObject<UStoreViewModel>(this);
		// Start in a "None" state so initial loads can be triggered on first access.
		StoreViewModel->AddStoreState(MolecularUITags::Store::State::None);

		StoreViewModelCollection->AddViewModelInstance(
			FMVVMViewModelContext(UStoreViewModel::StaticClass(), StoreViewModel_Name),
			StoreViewModel);
	}

	// Add default category tabs for the available items list.
	TArray<UCategoryViewModel*> CategoryTabViewModels_AvailableItems;
	for (const FCategoryTabDefinition& CategoryTab : DefaultCategoryTabs_AvailableItems)
	{
		UCategoryViewModel* CategoryVM = NewObject<UCategoryViewModel>(StoreViewModel);
		CategoryVM->SetCategoryTag(CategoryTab.CategoryTag);
		CategoryVM->SetUIData(CategoryTab.UIData);

		UE_MVVM_BIND_FIELD(UCategoryViewModel, CategoryVM, Interaction, OnItemCategoryInteractionChanged);
		CategoryTabViewModels_AvailableItems.Add(CategoryVM);
	}
	StoreViewModel->SetCategoryTabs_AvailableItems(CategoryTabViewModels_AvailableItems);
	if (!CategoryTabViewModels_AvailableItems.IsEmpty())
	{
		// Select the first category tab by default.
		SelectionViewModel_Store_Tabs->ToggleSelectViewModel(CategoryTabViewModels_AvailableItems[0]);
	}

	StoreViewModelCollection->AddViewModelInstance(
	FMVVMViewModelContext(USelectionViewModel::StaticClass(), SelectionViewModel_Store_Tabs_Name),
	SelectionViewModel_Store_Tabs);
	StoreViewModelCollection->AddViewModelInstance(
		FMVVMViewModelContext(USelectionViewModel::StaticClass(), SelectionViewModel_Store_Name),
		SelectionViewModel_Store);

	UE_MVVM_BIND_FIELD(UStoreViewModel, StoreViewModel, FilterText, OnFilterTextChanged);
	UE_MVVM_BIND_FIELD(UStoreViewModel, StoreViewModel, TransactionRequest, OnTransactionRequestChanged);
	UE_MVVM_BIND_FIELD(UStoreViewModel, StoreViewModel, bRefreshRequested, OnRefreshRequestedChanged);
}

void UStoreModel::DeinitializeModel_Implementation()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	UE_MVVM_UNBIND_FIELD(StoreViewModel, FilterText);
	UE_MVVM_UNBIND_FIELD(StoreViewModel, TransactionRequest);
	UE_MVVM_UNBIND_FIELD(StoreViewModel, bRefreshRequested);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	if (IsValid(StoreViewModelCollection))
	{
		StoreViewModelCollection->RemoveAllViewModelInstance(StoreViewModel);
		StoreViewModelCollection->RemoveAllViewModelInstance(SelectionViewModel_Store);
		StoreViewModelCollection->RemoveAllViewModelInstance(SelectionViewModel_Store_Tabs);
	}

	// Unbind any field notifications from the ItemViewModel.
	for (const TTuple<FName, TObjectPtr<UItemViewModel>>& Pair : ItemViewModelCache)
	{
		UItemViewModel* ItemVM = Pair.Value;
		if (IsValid(ItemVM))
		{
			UE_MVVM_UNBIND_FIELD(ItemVM, Interaction);
			for (UCategoryViewModel* CategoryVM : ItemVM->GetCategoryViewModels())
			{
				UE_MVVM_UNBIND_FIELD(CategoryVM, Interaction);
			}
		}
	}

	for (UCategoryViewModel* CategoryVM : StoreViewModel->GetCategoryTabs_AvailableItems())
	{
		UE_MVVM_UNBIND_FIELD(CategoryVM, Interaction);
	}

	StoreViewModel = nullptr;

	ItemViewModelCache.Empty();
	CachedStoreItems.Empty();

	StoreDataProviderInterface = nullptr;

	Super::DeinitializeModel_Implementation();
}
// End UMolecularModelBase interface.

// Begin IViewModelProvider implementation.
UMVVMViewModelBase* UStoreModel::GetViewModel_Implementation(FMVVMViewModelContext ViewModelContext)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	UMVVMViewModelBase* ViewModel = StoreViewModelCollection->FindViewModelInstance(ViewModelContext);
	if (!IsValid(ViewModel))
	{
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] No matching ViewModel found for context: %s"), 
	__FUNCTION__, *ViewModelContext.ContextName.ToString());
		return nullptr;
	}

	if (ViewModel == StoreViewModel)
	{
		if (StoreViewModel->HasStoreState(MolecularUITags::Store::State::None))
		{
			StoreViewModel->RemoveStoreState(MolecularUITags::Store::State::None);

			// Load stuff on initial open
			RefreshStoreData();
		}
	}

	return ViewModel;
}
// End IViewModelProvider implementation.

/* Field Notification Handlers */
void UStoreModel::OnFilterTextChanged_Implementation(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);
	FilterAvailableStoreItems();
}

void UStoreModel::OnSelectedCategoriesChanged_Implementation(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);
	FilterAvailableStoreItems();
}

void UStoreModel::OnTransactionRequestChanged_Implementation(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	const FTransactionRequest& TransactionRequest = InStoreViewModel->GetTransactionRequest();

	// Only process valid requests.
	// When the request is processed and cleared, this will be called again with an empty request,
	// so we can safely ignore that case.
	if (!TransactionRequest.IsValid() || !Field.IsValid())
	{
		return;
	}

	// If the store isn't ready, reset the request and exit early.
	if (!InStoreViewModel->HasStoreState(MolecularUITags::Store::State::Ready))
	{
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Store not ready. Failing transaction request for %s"), __FUNCTION__,
			   *TransactionRequest.ToString());

		InStoreViewModel->SetTransactionRequest(FTransactionRequest());
		InStoreViewModel->SetErrorMessage(FText::FromString("Store not ready. Please try again later."));
		InStoreViewModel->AddStoreState(MolecularUITags::Store::State::Error);
		return;
	}

	switch (InStoreViewModel->GetTransactionType())
	{
	case ETransactionType::None:
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] No transaction type set for request: %s"), __FUNCTION__,
			   *TransactionRequest.ToString());
		break;
	case ETransactionType::Purchase:
		LazyPurchaseItem(TransactionRequest);
		break;
	case ETransactionType::Sell:
		LazySellItem(TransactionRequest);
		break;
	}

	SelectionViewModel_Store->ClearSelection();
	SelectionViewModel_Store->ClearPreview();
}

void UStoreModel::OnRefreshRequestedChanged_Implementation(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	if (InStoreViewModel->GetRefreshRequested())
	{
		InStoreViewModel->SetRefreshRequested(false); // Reset the flag

		RefreshStoreData();
		
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Refresh requested. Reloading store data."), __FUNCTION__);
	}
}

void UStoreModel::OnItemInteractionChanged_Implementation(UItemViewModel* InItemVM, FFieldNotificationId Field)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	if (!ensure(IsValid(InItemVM)))
	{
		return;
	}

	const FInteractionState& Interaction = InItemVM->GetInteraction();
	if (!Interaction.IsValid())
	{
		return; // No valid interaction to process
	}
	
	const FString& SourceName = Interaction.Source.ToString();

	// Handle the interaction based on its type
	switch (Interaction.Type)
	{
	case EStatefulInteraction::Hovered:
		{
			const FString& ItemName = InItemVM->GetItemData().UIData.DisplayName.ToString();
			StoreViewModel->SetStatusMessage(FText::Format(
				FText::FromString("Previewing item: {0} (from {1})"),
				FText::FromString(ItemName), FText::FromString(SourceName)));
			SelectionViewModel_Store->PreviewViewModel(InItemVM);
			break;
		}
	case EStatefulInteraction::Unhovered:
		{
			StoreViewModel->SetStatusMessage(FText::GetEmpty());
			SelectionViewModel_Store->PreviewViewModel(SelectionViewModel_Store->GetLastSelectedViewModel());
			break;
		}
	case EStatefulInteraction::Clicked:
		{
			const FString& ItemName = InItemVM->GetItemData().UIData.DisplayName.ToString();
			StoreViewModel->SetStatusMessage(FText::Format(
				FText::FromString("Clicked on item: {0} (from {1})"),
				FText::FromString(ItemName), FText::FromString(SourceName)));

			// Avoid mixing owned items with the store's available items in the selection.
			const UItemViewModel* LastSelectedVM = Cast<UItemViewModel>(SelectionViewModel_Store->GetLastSelectedViewModel());
			if (IsValid(LastSelectedVM))
			{
				if (LastSelectedVM->GetItemData().bIsOwned != InItemVM->GetItemData().bIsOwned)
				{
					SelectionViewModel_Store->ClearSelection();
				}
			}
			SelectionViewModel_Store->ToggleSelectViewModel(InItemVM);
			
			if (InItemVM->GetItemData().bIsOwned)
			{
				// Passes the "client-side" check that the item can be sold.
				StoreViewModel->SetTransactionType(ETransactionType::Sell);
			}
			else if (InItemVM->GetItemData().Cost <= StoreViewModel->GetPlayerCurrency())
			{
				// Passes the "client-side" check that the item can be purchased.
				StoreViewModel->SetTransactionType(ETransactionType::Purchase);
			}
			else
			{
				// This is a valid state, there simply isn't any transaction available for this item.
				StoreViewModel->SetTransactionType(ETransactionType::None);
			}
			break;
		}
		
	default:
		break;
	}

	// Reset the interaction state after processing
	InItemVM->ClearInteraction();
}

void UStoreModel::OnItemCategoryInteractionChanged_Implementation(UCategoryViewModel* InCategoryVM, FFieldNotificationId Field)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	if (!ensure(IsValid(InCategoryVM)))
	{
		return;
	}

	const FInteractionState& Interaction = InCategoryVM->GetInteraction();
	if (!Interaction.IsValid())
	{
		return; // No valid interaction to process
	}

	const FString& CategoryTag = InCategoryVM->GetCategoryTag().ToString();

	// Handle the interaction based on its type
	switch (Interaction.Type)
	{
	case EStatefulInteraction::None:
		break;
	case EStatefulInteraction::Hovered:
		break;
	case EStatefulInteraction::Unhovered:
		{
			StoreViewModel->SetStatusMessage(FText::GetEmpty());
		}
		break;
	case EStatefulInteraction::Clicked:
		{
			if (Interaction.Source.MatchesTag(MolecularUITags::InteractionSource::TabList))
			{
				SelectionViewModel_Store_Tabs->ToggleSelectViewModel(InCategoryVM);
				SelectionViewModel_Store->ClearSelection(); // Switching tabs invalidates the current selection.

				StoreViewModel->SetStatusMessage(FText::Format(
					FText::FromString("{0} category: {1}"),
					FText::FromString(CategoryTag),
					FText::FromString(Interaction.ToString())
				));
				FilterAvailableStoreItems();
			}
		}
		break;
	}
		// Reset the interaction state after processing
	InCategoryVM->ClearInteraction();
}

/* Lazy Loading Functions */
void UStoreModel::LazyLoadStoreItems_Implementation()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	SCOPED_STORE_STATE(LoadingScope, StoreViewModel, MolecularUITags::Store::State::Loading::Items);

	auto OnSuccess = [this, LoadingScope](const TArray<FStoreItem>& Items, const FText& Status)
	{
		(void)LoadingScope;
		CachedStoreItems = Items;
		TArray<TObjectPtr<UItemViewModel>> StoreItems;
		StoreItems.Reserve(Items.Num());

		for (const FStoreItem& ItemData : Items)
		{
			UItemViewModel* ItemVM = GetOrCreateItemViewModel(ItemData);
			StoreItems.AddUnique(ItemVM);
		}

		StoreViewModel->SetAvailableItems(StoreItems);
		FilterAvailableStoreItems();
		StoreViewModel->SetStatusMessage(Status);
	};

	auto OnFailure = [this, LoadingScope](const FText& Error)
	{
		(void)LoadingScope;
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Failure loading store items."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(Error);
		StoreViewModel->AddStoreState(MolecularUITags::Store::State::Error);
	};

	if (StoreDataProviderInterface)
	{
		StoreDataProviderInterface->FetchStoreItems(OnSuccess, OnFailure);
	}
}

void UStoreModel::LazyLoadOwnedItems_Implementation()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	SCOPED_STORE_STATE(LoadingScope, StoreViewModel, MolecularUITags::Store::State::Loading::OwnedItems);

	auto OnSuccess = [this, LoadingScope](const TArray<FStoreItem>& Items, const FText& Status)
	{
		(void)LoadingScope;
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
		(void)LoadingScope;
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Failure loading owned items."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(Error);
		StoreViewModel->AddStoreState(MolecularUITags::Store::State::Error);
	};

	if (StoreDataProviderInterface)
	{
		StoreDataProviderInterface->FetchOwnedItems(OnSuccess, OnFailure);
	}
}

void UStoreModel::LazyLoadStoreCurrency_Implementation()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	SCOPED_STORE_STATE(LoadingScope, StoreViewModel, MolecularUITags::Store::State::Loading::Currency);

	auto OnSuccess = [this, LoadingScope](int32 Currency, const FText& Status)
	{
		(void)LoadingScope;
		StoreViewModel->SetPlayerCurrency(Currency);
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Player currency loaded: %d"), __FUNCTION__, Currency);
		StoreViewModel->SetStatusMessage(Status);
	};

	auto OnFailure = [this, LoadingScope](const FText& Error)
	{
		(void)LoadingScope;
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Failure loading store currency."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(Error);
		StoreViewModel->AddStoreState(MolecularUITags::Store::State::Error);
	};

	if (StoreDataProviderInterface)
	{
		StoreDataProviderInterface->FetchPlayerCurrency(OnSuccess, OnFailure);
	}
}

void UStoreModel::LazyPurchaseItem_Implementation(const FTransactionRequest& PurchaseRequest)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	SCOPED_STORE_STATE(PurchaseScope, StoreViewModel, MolecularUITags::Store::State::Purchasing);

	auto OnSuccess = [this, PurchaseScope](const FText& Status)
	{
		(void)PurchaseScope;
		// Clear the transaction request and type after a successful purchase.
		StoreViewModel->SetTransactionRequest(FTransactionRequest());
		StoreViewModel->SetTransactionType(ETransactionType::None);

		RefreshStoreData();

		StoreViewModel->SetStatusMessage(Status);
	};

	auto OnFailure = [this, PurchaseScope](const FText& Error)
	{
		(void)PurchaseScope;
		StoreViewModel->AddStoreState(MolecularUITags::Store::State::Error);
		StoreViewModel->SetErrorMessage(Error);
	};

	if (StoreDataProviderInterface)
	{
		StoreDataProviderInterface->PurchaseItem(PurchaseRequest, OnSuccess, OnFailure);
	}
}

void UStoreModel::LazySellItem_Implementation(const FTransactionRequest& TransactionRequest)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	SCOPED_STORE_STATE(SellScope, StoreViewModel, MolecularUITags::Store::State::Selling);

	auto OnSuccess = [this, SellScope](const FText& Status)
	{
		(void)SellScope;
		StoreViewModel->SetTransactionRequest(FTransactionRequest());
		StoreViewModel->SetTransactionType(ETransactionType::None);

		RefreshStoreData();

		StoreViewModel->SetStatusMessage(Status);
	};

	auto OnFailure = [this, SellScope](const FText& Error)
	{
		(void)SellScope;
		StoreViewModel->AddStoreState(MolecularUITags::Store::State::Error);
		StoreViewModel->SetErrorMessage(Error);
	};

	if (StoreDataProviderInterface)
	{
		StoreDataProviderInterface->SellItem(TransactionRequest, OnSuccess, OnFailure);
	}
}

/* Utility Functions */
void UStoreModel::FilterAvailableStoreItems_Implementation()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	if (CachedStoreItems.IsEmpty())
	{
		return;
	}

	const FString& FilterText = StoreViewModel->GetFilterText();
	const TArray<UInteractiveViewModelBase*>& SelectedCategories_AvailableItems = SelectionViewModel_Store_Tabs->GetSelectedViewModels();

	TArray<TObjectPtr<UItemViewModel>> FilteredItems;
	FilteredItems.Reserve(CachedStoreItems.Num());

	for (const FStoreItem& ItemData : CachedStoreItems)
	{
		if (ItemData.bIsOwned)
		{
			continue; // Skip owned items in the available items list.
		}

		// Text filter pass
		const bool bTextFilterMatch = FilterText.IsEmpty() || ItemData.UIData.DisplayName.ToString().Contains(FilterText);
		if (!bTextFilterMatch)
		{
			continue;
		}

		// Category filter pass
		bool bCategoryMatchFound = SelectedCategories_AvailableItems.IsEmpty();
		if (!bCategoryMatchFound)
		{
			for (UInteractiveViewModelBase* SelectedVM : SelectedCategories_AvailableItems)
			{
				const UCategoryViewModel* SelectedCategoryVM = Cast<UCategoryViewModel>(SelectedVM);
				if (!IsValid(SelectedCategoryVM))
				{
					continue;
				}

				// If "All" is selected, the item passes the filter.
				if (SelectedCategoryVM->IsAll())
				{
					bCategoryMatchFound = true;
					break;
				}

				// Check if the item's tags match the selected category tag.
				const FGameplayTag& SelectedCategoryTag = SelectedCategoryVM->GetCategoryTag();
				if (SelectedCategoryTag.IsValid() && ItemData.Categories.HasTag(SelectedCategoryTag))
				{
					bCategoryMatchFound = true;
					break; // Found a match, no need to check other categories.
				}
			}
		}
    
		// If after checking all categories, no match was found, skip this item.
		if (!bCategoryMatchFound)
		{
			continue;
		}

		UItemViewModel* ItemVM = GetOrCreateItemViewModel(ItemData);
		FilteredItems.Add(ItemVM);
	}

	StoreViewModel->SetAvailableItems(FilteredItems);
}

void UStoreModel::RefreshStoreData_Implementation()
{
	// Clear any existing error message
	StoreViewModel->SetErrorMessage(FText::GetEmpty());
	StoreViewModel->RemoveStoreState(MolecularUITags::Store::State::Error);

	// Refresh the store data
	LazyLoadStoreItems();
	LazyLoadOwnedItems();
	LazyLoadStoreCurrency();

	SelectionViewModel_Store->ClearPreview();
	SelectionViewModel_Store->ClearSelection();
	SelectionViewModel_Store_Tabs->ClearPreview();
}

UItemViewModel* UStoreModel::GetOrCreateItemViewModel(const FStoreItem& ItemData)
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
	UE_MVVM_BIND_FIELD(UItemViewModel, NewItemVM, Interaction, OnItemInteractionChanged);

	// Add the new ViewModel to the cache for future reuse.
	ItemViewModelCache.Add(ItemData.ItemId, NewItemVM);

	// Set the category view models if needed.
	// This generates the category ViewModels that are associated with this item.
	// They can be used to represent category tabs to filter items in the store or to show item details, or any other UI element that needs to display categories.
	TArray<TObjectPtr<UCategoryViewModel>> CategoryVMs;
	for (const FGameplayTag& CategoryTag : ItemData.Categories)
	{
		UCategoryViewModel* CategoryVM = NewObject<UCategoryViewModel>(NewItemVM);
		CategoryVM->SetCategoryTag(CategoryTag);

		// TODO: Look up UI data mapped to the category tag in project settings
		FStandardUIData CategoryUIData;
		CategoryUIData.DisplayName = FText::FromString(CategoryTag.GetTagName().ToString());
		CategoryUIData.Description = FText::FromString(FString::Printf(TEXT("Category: %s"), *CategoryTag.GetTagName().ToString()));
		CategoryUIData.Icon = FSlateBrush();
		CategoryVM->SetUIData(CategoryUIData);
	
		UE_MVVM_BIND_FIELD(UCategoryViewModel, CategoryVM, Interaction, OnItemCategoryInteractionChanged);
		CategoryVMs.Add(CategoryVM);
		if (bAutoGenerateCategoriesFromItems)
		{
			StoreViewModel->AddCategory_AvailableItems(CategoryVM);
		}
	}
	NewItemVM->SetCategoryViewModels(CategoryVMs);

	return NewItemVM;
}
