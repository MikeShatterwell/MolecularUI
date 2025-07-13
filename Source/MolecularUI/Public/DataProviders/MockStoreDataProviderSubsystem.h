// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include "Interfaces/IStoreDataProvider.h"
#include "MockStoreDataProviderSubsystem.generated.h"

/*
 * The mock data provider simulates a store backend by generating dummy data
 * and simulating asynchronous operations with random delays and failure chances.
 */
UCLASS()
class UMockStoreDataProviderSubsystem : public UGameInstanceSubsystem, public IStoreDataProvider
{
	GENERATED_BODY()

public:
	// Begin IStoreDataProvider implementation
	virtual void FetchStoreItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
								 TFunction<void(const FText&)> OnFailure) override;
	virtual void FetchOwnedItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
								 TFunction<void(const FText&)> OnFailure) override;
	virtual void FetchPlayerCurrency(TFunction<void(int32, const FText&)> OnSuccess,
									 TFunction<void(const FText&)> OnFailure) override;
	virtual void PurchaseItem(const FTransactionRequest& Request,
							  TFunction<void(const FText&)> OnSuccess,
							  TFunction<void(const FText&)> OnFailure) override;
	virtual void SellItem(const FTransactionRequest& Request,
							  TFunction<void(const FText&)> OnSuccess,
							  TFunction<void(const FText&)> OnFailure) override;
	// End IStoreDataProvider implementation

protected:
	void CreateDummyStoreData(TFunction<void()> OnComplete);
	void CreateDummyOwnedStoreData(TFunction<void()> OnComplete);
	void CreateDummyPlayerCurrency();


	/**
	 * Asynchronously loads items from a given data table into the target array, optionally marking them as owned.
	 *
	 * @param DataTable The soft reference to the data table from which items will be loaded.
	 * @param TargetArray The array where loaded items will be stored.
	 * @param OnComplete A callback function that will be executed when the loading process is complete.
	 */
	void LoadItemsFromDataTable(
	const TSoftObjectPtr<UDataTable>& DataTable,
	TArray<FStoreItem>& TargetArray,
	TFunction<void()> OnComplete) const;

	TArray<FStoreItem> BackendStoreItems;
	TArray<FStoreItem> BackendOwnedStoreItems;
	int32 BackendPlayerCurrency = INDEX_NONE;

	FTimerHandle ItemLoadHandle;
	FTimerHandle OwnedItemLoadHandle;
	FTimerHandle CurrencyLoadHandle;
	FTimerHandle TransactionHandle;
	FTimerHandle SellHandle;
	
	bool bDummyStoreDataInitialized = false;
	bool bDummyOwnedDataInitialized = false;
	bool bDummyPlayerCurrencyInitialized = false;

	bool bIsLoadingStoreItems = false;
	bool bIsLoadingOwnedItems = false;
};
