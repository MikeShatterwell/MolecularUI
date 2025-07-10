// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <Subsystems/GameInstanceSubsystem.h>

#include "IStoreViewModelProvider.h"
#include "MolecularTypes.h"
#include "DataProviders/IStoreDataProvider.h"
#include "StoreModelSubsystem.generated.h"

class UCategoryViewModel;
class UMVVMViewModelBase;
class UItemViewModel;
class UStoreViewModel;

UCLASS(Blueprintable, Abstract, DisplayName = "Store Mock Data Subsystem")
class UStoreModelSubsystem : public UGameInstanceSubsystem, public IStoreViewModelProvider
{
	GENERATED_BODY()

public:
	// Begin USubsystem interface.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem interface.

	// Begin IStoreViewModelProvider implementation.
	virtual UStoreViewModel* GetStoreViewModel_Implementation() override;
	// End IStoreViewModelProvider implementation.

protected:
	// Reacts to changes in the ViewModel's properties.
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model Subsystem | Stateful Communication")
	void OnFilterTextChanged(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model Subsystem | Stateful Communication")
	void OnSelectedCategoriesChanged(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model Subsystem | Stateful Communication")
	void OnTransactionRequestChanged(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model Subsystem | Stateful Communication")
	void OnRefreshRequestedChanged(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field);

	UFUNCTION(BlueprintNativeEvent, Category = "Store Model Subsystem | Stateful Communication")
	void OnItemInteractionChanged(UItemViewModel* InItemVM, FFieldNotificationId Field);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model Subsystem | Stateful Communication")
	void OnItemCategoryInteractionChanged(UCategoryViewModel* InCategoryVM, FFieldNotificationId Field);

	// Simulates sending and receiving data asynchronously.
	void LazyLoadStoreItems();
	void LazyLoadStoreCurrency();
	void LazyLoadOwnedItems();
	void LazyPurchaseItem(const FTransactionRequest& PurchaseRequest);
	void LazySellItem(const FTransactionRequest& TransactionRequest);

	// Filters the cached store items based on the filter text and selected categories.
	void FilterAvailableStoreItems();

	// Lazy loads the store data from the provider.
	void RefreshStoreData();

	// The single, authoritative instance of the Store ViewModel.
	UPROPERTY(Transient)
	TObjectPtr<UStoreViewModel> StoreViewModel = nullptr;
	
	// Cache for item view models to reduce UObject churn.
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UItemViewModel>> ItemViewModelCache;

	// Cached list of store items used for filtering without repeated backend calls.
	UPROPERTY(Transient)
	TArray<FStoreItem> CachedStoreItems;

	// Cached interface pointer to the provider instance.
	TScriptInterface<IStoreDataProvider> StoreDataProviderInterface;
	
	/**
	 * Centralized factory method for ItemViewModels.
	 * Retrieves an ItemViewModel from the cache or creates a new one if it doesn't exist.
	 * Ensures the ViewModel's data is up-to-date with the provided FStoreItem.
	 * @param ItemData The backend data for the item.
	 * @return A valid UItemViewModel pointer.
	 */
	UItemViewModel* GetOrCreateItemViewModel(const FStoreItem& ItemData);
};
