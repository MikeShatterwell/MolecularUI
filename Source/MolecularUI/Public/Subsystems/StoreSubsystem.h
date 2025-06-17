// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <Subsystems/GameInstanceSubsystem.h>

#include "IStoreViewModelProvider.h"
#include "MolecularTypes.h"
#include "DataProviders/StoreDataProvider.h"
#include "DataProviders/MockStoreDataProvider.h"
#include "StoreSubsystem.generated.h"

class UItemViewModel;
class UStoreViewModel;

UCLASS(Blueprintable, DisplayName = "Store Mock Data Subsystem")
class UStoreSubsystem : public UGameInstanceSubsystem, public IStoreViewModelProvider
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Break out of the error state by clearing the error message and resetting the state. */
	UFUNCTION(BlueprintCallable, Category = "Store Subsystem")
	void BreakErrorState();

	// IStoreViewModelProvider implementation.
	virtual UStoreViewModel* GetStoreViewModel_Implementation() override;

protected:
	// Reacts to changes in the ViewModel's properties.
	void OnFilterTextChanged(UObject* Object, UE::FieldNotification::FFieldId Field);
	void OnTransactionRequestChanged(UObject* Object, UE::FieldNotification::FFieldId Field);
	void OnRefreshRequestedChanged(UObject* Object, UE::FieldNotification::FFieldId Field);
	void OnItemInteractionChanged(UObject* Object, UE::FieldNotification::FFieldId Field);

	// Simulates sending and receiving data asynchronously.
	void LazyLoadStoreItems();
	void LazyLoadStoreCurrency();
	void LazyLoadOwnedItems();
	void LazyPurchaseItem(const FTransactionRequest& PurchaseRequest);

	void FilterAvailableStoreItems(const FString& FilterText);



	// The single, authoritative instance of the Store ViewModel.
	UPROPERTY(Transient)
	TObjectPtr<UStoreViewModel> StoreViewModel = nullptr;
	
	// Cache for item view models to reduce UObject churn.
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UItemViewModel>> ItemViewModelCache;

	/**
	 * Class used to instantiate the data provider. Replace this with your
	 * own UObject class that implements IStoreDataProvider (e.g.
	 * UBackendStoreDataProvider) to integrate a real backend without
	 * modifying the subsystem.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Store", meta = (AllowAbstract = true))
	TSubclassOf<UObject> DataProviderClass = UMockStoreDataProvider::StaticClass();

	/** Instance of the provider created from DataProviderClass. */
	UPROPERTY(Transient)
	TObjectPtr<UObject> DataProviderObject = nullptr;

	/** Cached interface pointer to the provider instance. */
	TScriptInterface<IStoreDataProvider> DataProvider;


	/**
	 * Centralized factory method for ItemViewModels.
	 * Retrieves an ItemViewModel from the cache or creates a new one if it doesn't exist.
	 * Ensures the ViewModel's data is up-to-date with the provided FStoreItem.
	 * @param ItemData The backend data for the item.
	 * @return A valid UItemViewModel pointer.
	 */
	UItemViewModel* GetOrCreateItemViewModel(const FStoreItem& ItemData);
};
