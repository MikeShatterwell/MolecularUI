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
	// Setters and Getters
	void SetPlayerCurrency(const int32 InCurrency) { UE_MVVM_SET_PROPERTY_VALUE(PlayerCurrency, InCurrency); }
	int32 GetPlayerCurrency() const { return PlayerCurrency; }

	void SetSelectedItem(UItemViewModel* InViewModel) { UE_MVVM_SET_PROPERTY_VALUE(SelectedItem, InViewModel); }
	UItemViewModel* GetSelectedItem() const { return SelectedItem; }

	void SetPurchaseRequest(const FPurchaseRequest InRequest) { UE_MVVM_SET_PROPERTY_VALUE(PurchaseRequest, InRequest); }
	FPurchaseRequest GetPurchaseRequest() const { return PurchaseRequest; }

	void SetAvailableItems(const TArray<UItemViewModel*>& InItems) { UE_MVVM_SET_PROPERTY_VALUE(AvailableItems, InItems); }
	const TArray<UItemViewModel*>& GetAvailableItems() const { return AvailableItems; }

	void SetOwnedItems(const TArray<UItemViewModel*>& InItems) { UE_MVVM_SET_PROPERTY_VALUE(OwnedItems, InItems); }
	const TArray<TObjectPtr<UItemViewModel>>& GetOwnedItems() const { return OwnedItems; }

	void SetStoreState(const EStoreState InState) { UE_MVVM_SET_PROPERTY_VALUE(StoreState, InState); }
	EStoreState GetStoreState() const { return StoreState; }

	void SetFilterText(const FString& InFilterText) { UE_MVVM_SET_PROPERTY_VALUE(FilterText, InFilterText); }
	FString GetFilterText() const { return FilterText; }

	void SetErrorMessage(const FText& InErrorMessage) { UE_MVVM_SET_PROPERTY_VALUE(ErrorMessage, InErrorMessage); }
	FText GetErrorMessage() const { return ErrorMessage; }

protected:
	/* These are the "Data Properties" that the ViewModel will expose to the View. */
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	int32 PlayerCurrency;


	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	TArray<TObjectPtr<UItemViewModel>> AvailableItems;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	TArray<TObjectPtr<UItemViewModel>> OwnedItems;


	/* These are the "Intent Channels" that allow the ViewModel to communicate with the Model. */
	// The UI sets this when the user hovers or selects an item. Other UI elements can bind to this to show details.
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel | Intent")
	TObjectPtr<UItemViewModel> SelectedItem;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel | Intent")
	FPurchaseRequest PurchaseRequest;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	FString FilterText;

	/* These are the "Stateful Communication" properties that allow the Model to communicate back to the ViewModel. */
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	EStoreState StoreState = EStoreState::None;
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	FText ErrorMessage = FText::GetEmpty();
};