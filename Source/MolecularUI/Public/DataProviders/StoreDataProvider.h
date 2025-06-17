#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>

#include "MolecularTypes.h"
#include "StoreDataProvider.generated.h"

struct FTransactionRequest;

UINTERFACE(MinimalAPI, Blueprintable)
class UStoreDataProvider : public UInterface
{
    GENERATED_BODY()
};

class IStoreDataProvider
{
    GENERATED_BODY()

public:
    virtual void FetchStoreItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
                                 TFunction<void(const FText&)> OnFailure) = 0;

    virtual void FetchOwnedItems(TFunction<void(const TArray<FStoreItem>&, const FText&)> OnSuccess,
                                 TFunction<void(const FText&)> OnFailure) = 0;

    virtual void FetchPlayerCurrency(TFunction<void(int32, const FText&)> OnSuccess,
                                     TFunction<void(const FText&)> OnFailure) = 0;

    virtual void PurchaseItem(const FTransactionRequest& Request,
                              TFunction<void(const FText&)> OnSuccess,
                              TFunction<void(const FText&)> OnFailure) = 0;
};
