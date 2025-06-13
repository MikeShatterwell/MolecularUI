#pragma once

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "StoreTypes.h"

#include "StoreViewModel.generated.h"

class UItemViewModel;

UCLASS(BlueprintType)
class UStoreViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	int32 PlayerCurrency;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	TArray<TObjectPtr<UItemViewModel>> AvailableItems;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	TArray<TObjectPtr<UItemViewModel>> OwnedItems;

	/* These are the "Intent Channels" that allow the ViewModel to communicate with the Model (Subsystem). */

	// UI sets this when the user selects an item
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	FPurchaseRequest PurchaseRequest;

	UPROPERTY(BlueprintReadWrite, Setter, FieldNotify, meta = (AllowPrivateAccess = true))
	FString FilterText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Store ViewModel")
	EStoreState StoreState = EStoreState::None;

	// Setters and Getters
	void SetPlayerCurrency(const int32 InCurrency) { UE_MVVM_SET_PROPERTY_VALUE(PlayerCurrency, InCurrency); }
	int32 GetPlayerCurrency() const { return PlayerCurrency; }

	void SetAvailableItems(const TArray<FStoreItem>& InItems) { UE_MVVM_SET_PROPERTY_VALUE(AvailableItems, InItems); }
	const TArray<UItemViewModel*>& GetAvailableItems() const { return AvailableItems; }

	void SetPurchaseRequest(const FPurchaseRequest InRequest) { UE_MVVM_SET_PROPERTY_VALUE(PurchaseRequest, InRequest); }
	FPurchaseRequest GetPurchaseRequest() const { return PurchaseRequest; }
};