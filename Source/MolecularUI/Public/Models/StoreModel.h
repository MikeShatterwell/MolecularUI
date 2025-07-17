// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <Subsystems/GameInstanceSubsystem.h>

#include "Interfaces/IStoreDataProvider.h"
#include "MolecularTypes.h"
#include "Models/MolecularModelBase.h"
#include "StoreModel.generated.h"

class UMVVMViewModelCollectionObject;
struct FMVVMViewModelContext;
class USelectionViewModel;
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
	virtual UMVVMViewModelBase* GetViewModel_Implementation(FMVVMViewModelContext ViewModelContext) override;
	// End IViewModelProvider override.

	// TODO: Add editor-time checking for selection vms

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
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void LazyLoadStoreItems();

	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void LazyLoadStoreCurrency();

	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void LazyLoadOwnedItems();

	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void LazyPurchaseItem(const FTransactionRequest& PurchaseRequest);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void LazySellItem(const FTransactionRequest& TransactionRequest);

	// Filters the cached store items based on the filter text and selected categories.
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void FilterAvailableStoreItems();

	// Lazy loads the store data from the provider.
	UFUNCTION(BlueprintNativeEvent, Category = "Store Model")
	void RefreshStoreData();


	// Predefined categories that are always added to the store model.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store Model|CategoryTabs")
	TArray<FCategoryTabDefinition> DefaultCategoryTabs_AvailableItems;

	// Whether to automatically generate categories based on the store items.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store Model|CategoryTabs")
	bool bAutoGenerateCategoriesFromItems = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store Model")
	FName SelectionViewModel_Store_Name = FName(TEXT("SelectionViewModel_Store"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store Model|CategoryTabs")
	FName SelectionViewModel_Store_Tabs_Name = FName(TEXT("SelectionViewModel_Store_Tabs"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store Model|CategoryTabs")
	FName StoreViewModel_Name = FName(TEXT("StoreViewModel"));

	
	// Manages store item selection and reflects the current selection state.
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Store Model|Selection")
	TObjectPtr<USelectionViewModel> SelectionViewModel_Store = nullptr;

	// Manages category tab selection and reflects the current selection state.
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Store Model|Selection")
	TObjectPtr<USelectionViewModel> SelectionViewModel_Store_Tabs = nullptr;
	
	// The single, authoritative instance of the Store ViewModel.
	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UStoreViewModel> StoreViewModel = nullptr;

	// Holds all ViewModels for the store Model
	UPROPERTY(Transient)
	TObjectPtr<UMVVMViewModelCollectionObject> StoreViewModelCollection = nullptr;
	

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
