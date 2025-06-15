#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "IStoreViewModelProvider.h"
#include "MolecularTypes.h"
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

	// IStoreViewModelProvider implementation.
	virtual UStoreViewModel* GetStoreViewModel_Implementation() override;

protected:
	// Reacts to changes in the ViewModel's properties.
	void OnFilterTextChanged(UObject* Object, UE::FieldNotification::FFieldId Field);
	void OnPurchaseRequestChanged(UObject* Object, UE::FieldNotification::FFieldId Field);
	void OnItemInteractionChanged(UObject* Object, UE::FieldNotification::FFieldId Field);

	// Simulates sending and receiving data asynchronously.
	void LazyLoadStoreItems();
	void LazyLoadStoreCurrency();
	void LazyLoadOwnedItems();
	void LazyPurchaseItem(const FPurchaseRequest& PurchaseRequest);

	/** Attempts to process the next queued purchase request when possible. */
	void ProcessPendingPurchaseRequests();

	// Helper methods to create dummy data for testing purposes.
	void CreateDummyStoreData();
	void CreateDummyOwnedStoreData();
	void CreateDummyPlayerCurrency();


	// The single, authoritative instance of the Store ViewModel.
	UPROPERTY(Transient)
	TObjectPtr<UStoreViewModel> StoreViewModel = nullptr;
	
	// Cache for item view models to reduce UObject churn.
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UItemViewModel>> ItemViewModelCache;

	/**
	 * Pending purchase requests queued when the store is not ready.
	 */
	UPROPERTY(Transient)
	TArray<FPurchaseRequest> PendingPurchaseRequests;

	UPROPERTY()
	int32 MaxPendingPurchaseRequestsCount = 5;

	/* The raw "model" data. In a real context, this would come from a database or a backend API. */
	UPROPERTY(Transient)
	TArray<FStoreItem> BackendStoreItems;

	UPROPERTY(Transient)
	TArray<FStoreItem> BackendOwnedStoreItems;

	UPROPERTY(Transient)
	int32 BackendPlayerCurrency = INDEX_NONE;


	/* Timers for simulating lazy loading and purchase operations. */
	FTimerHandle ItemLoadTimerHandle;
	FTimerHandle OwnedItemLoadTimerHandle;
	FTimerHandle CurrencyLoadTimerHandle;
	FTimerHandle ItemPurchaseTimerHandle;


	/**
	 * Centralized factory method for ItemViewModels.
	 * Retrieves an ItemViewModel from the cache or creates a new one if it doesn't exist.
	 * Ensures the ViewModel's data is up-to-date with the provided FStoreItem.
	 * @param ItemData The backend data for the item.
	 * @return A valid UItemViewModel pointer.
	 */
	UItemViewModel* GetOrCreateItemViewModel(const FStoreItem& ItemData);
};
