// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include "DataProviders/StoreDataProvider.h"
#include "MockStoreDataProvider.generated.h"

UCLASS()
class UMockStoreDataProvider : public UObject, public IStoreDataProvider
{
	GENERATED_BODY()

public:
	void InitializeProvider(UObject* InOuter);

	virtual void FetchStoreItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
								 TFunction<void(const FText&)> OnFailure) override;
	virtual void FetchOwnedItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
								 TFunction<void(const FText&)> OnFailure) override;
	virtual void FetchPlayerCurrency(TFunction<void(int32, const FText&)> OnSuccess,
									 TFunction<void(const FText&)> OnFailure) override;
	virtual void PurchaseItem(const FTransactionRequest& Request,
							  TFunction<void(const FText&)> OnSuccess,
							  TFunction<void(const FText&)> OnFailure) override;

protected:
	void CreateDummyStoreData();
	void CreateDummyOwnedStoreData();
	void CreateDummyPlayerCurrency();

	// Begin UObject overrides
	virtual UWorld* GetWorld() const override;
	// End UObject overrides
	
	TWeakObjectPtr<UWorld> OuterWorld;

	TArray<FStoreItem> BackendStoreItems;
	TArray<FStoreItem> BackendOwnedStoreItems;
	int32 BackendPlayerCurrency = INDEX_NONE;

	FTimerHandle ItemLoadHandle;
	FTimerHandle OwnedItemLoadHandle;
	FTimerHandle CurrencyLoadHandle;
	FTimerHandle PurchaseHandle;
};

