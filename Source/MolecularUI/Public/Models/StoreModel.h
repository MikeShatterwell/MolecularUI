// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <Subsystems/GameInstanceSubsystem.h>

#include "Interfaces/IStoreDataProvider.h"
#include "MolecularTypes.h"
#include "Models/MolecularModelBase.h"
#include "StoreModel.generated.h"

class UCategoryViewModel;
class UMVVMViewModelBase;
class UItemViewModel;
class UStoreViewModel;

UCLASS(DisplayName = "Store Model Base")
class UStoreModel : public UMolecularModelBase
{
	GENERATED_BODY()

public:
	// Begin UMolecularModelBase overrides.
	virtual void InitializeModel_Implementation(UWorld* World) override;
	virtual void DeinitializeModel_Implementation() override;
	// End UMolecularModelBase overrides.

	// Begin IViewModelProvider override.
	virtual UMVVMViewModelBase* GetViewModel_Implementation(TSubclassOf<UMVVMViewModelBase> ViewModelClass) override;
	// End IViewModelProvider override.

protected:
	// Reacts to changes in the ViewModel's properties.
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void OnFilterTextChanged(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void OnSelectedCategoriesChanged(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void OnTransactionRequestChanged(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void OnRefreshRequestedChanged(UStoreViewModel* InStoreViewModel, FFieldNotificationId Field);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void OnItemInteractionChanged(UItemViewModel* InItemVM, FFieldNotificationId Field);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void OnItemCategoryInteractionChanged(UCategoryViewModel* InCategoryVM, FFieldNotificationId Field);

	// Simulates sending and receiving data asynchronously.
	UFUNCTION(BlueprintCallable, Category = "Store Model")
	void LazyLoadStoreItems();

	UFUNCTION(BlueprintCallable, Category = "Store Model")
	void LazyLoadStoreCurrency();

	UFUNCTION(BlueprintCallable, Category = "Store Model")
	void LazyLoadOwnedItems();

	UFUNCTION(BlueprintCallable, Category = "Store Model")
	void LazyPurchaseItem(const FTransactionRequest& PurchaseRequest);
	
	UFUNCTION(BlueprintCallable, Category = "Store Model")
	void LazySellItem(const FTransactionRequest& TransactionRequest);

	// Filters the cached store items based on the filter text and selected categories.
	UFUNCTION(BlueprintCallable, Category = "Store Model")
	void FilterAvailableStoreItems();

	// Lazy loads the store data from the provider.
	UFUNCTION(BlueprintCallable, Category = "Store Model")
	void RefreshStoreData();

	// The single, authoritative instance of the Store ViewModel.
	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UStoreViewModel> StoreViewModel = nullptr;
	
	// Cache for item view models to reduce UObject churn.
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UItemViewModel>> ItemViewModelCache;

	// Cached list of store items used for filtering without repeated backend calls.
	UPROPERTY(BlueprintReadWrite, Transient)
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
