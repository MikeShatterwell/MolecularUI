// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MolecularTypes.h"
#include "MolecularUITags.h"

#include "StoreViewModel.generated.h"

class UCategoryViewModel;
class UItemViewModel;

UCLASS(Blueprintable, DisplayName = "Store ViewModel")
class UStoreViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	// Setters and Getters
	void SetPlayerCurrency(const int32 InCurrency) { UE_MVVM_SET_PROPERTY_VALUE(PlayerCurrency, InCurrency); }
	int32 GetPlayerCurrency() const { return PlayerCurrency; }

	void SetSelectedItem(UItemViewModel* InViewModel) { UE_MVVM_SET_PROPERTY_VALUE(SelectedItem, InViewModel); }
	UItemViewModel* GetSelectedItem() const { return SelectedItem; }

	void SetPreviewedItem(UItemViewModel* InViewModel) { UE_MVVM_SET_PROPERTY_VALUE(PreviewedItem, InViewModel); }
	UItemViewModel* GetPreviewedItem() const { return PreviewedItem; }

	void SetTransactionRequest(const FTransactionRequest& InRequest) { UE_MVVM_SET_PROPERTY_VALUE(TransactionRequest, InRequest); }
	FTransactionRequest GetTransactionRequest() const { return TransactionRequest; }

	void SetSelectedCategories(const TArray<TObjectPtr<UCategoryViewModel>>& InCategories)
	{
		UE_MVVM_SET_PROPERTY_VALUE(SelectedCategories, InCategories);
	}

	const TArray<TObjectPtr<UCategoryViewModel>>& GetSelectedCategories() const { return SelectedCategories; }

	void SetTransactionType(const ETransactionType InType) { UE_MVVM_SET_PROPERTY_VALUE(TransactionType, InType); }
	ETransactionType GetTransactionType() const { return TransactionType; }

	void SetAvailableItems(const TArray<TObjectPtr<UItemViewModel>>& InItems) { UE_MVVM_SET_PROPERTY_VALUE(AvailableItems, InItems); }
	const TArray<TObjectPtr<UItemViewModel>>& GetAvailableItems() const { return AvailableItems; }

	void SetOwnedItems(const TArray<TObjectPtr<UItemViewModel>>& InItems) { UE_MVVM_SET_PROPERTY_VALUE(OwnedItems, InItems); }
	const TArray<TObjectPtr<UItemViewModel>>& GetOwnedItems() const { return OwnedItems; }

	void SetStoreStates(const FGameplayTagContainer& InStates) { UE_MVVM_SET_PROPERTY_VALUE(StoreStates, InStates); }
	const FGameplayTagContainer& GetStoreStates() const { return StoreStates; }

	void AddStoreState(const FGameplayTag& State)
	{
		FGameplayTagContainer NewStates = StoreStates;
		NewStates.RemoveTag(MolecularUITags::Store::State::Ready);
		NewStates.AddTag(State);
		SetStoreStates(NewStates);
	}

	void RemoveStoreState(const FGameplayTag& State)
	{
		FGameplayTagContainer NewStates = StoreStates;
		NewStates.RemoveTag(State);
		if (NewStates.Num() == 0)
		{
			NewStates.AddTag(MolecularUITags::Store::State::Ready);
		}
		SetStoreStates(NewStates);
	}

	UFUNCTION(BlueprintPure, Category = "Store ViewModel")
	bool HasStoreState(const FGameplayTag& State) const
	{
		return StoreStates.HasTagExact(State);
	}

	void SetFilterText(const FString& InFilterText) { UE_MVVM_SET_PROPERTY_VALUE(FilterText, InFilterText); }
	FString GetFilterText() const { return FilterText; }

	UFUNCTION(BlueprintCallable, Category = "Store ViewModel")
	void SetRefreshRequested(const bool bInRefreshRequested) { UE_MVVM_SET_PROPERTY_VALUE(bRefreshRequested, bInRefreshRequested); }
	UFUNCTION(BlueprintPure, Category = "Store ViewModel")
	bool GetRefreshRequested() const { return bRefreshRequested; }

	void SetErrorMessage(const FText& InErrorMessage) { UE_MVVM_SET_PROPERTY_VALUE(ErrorMessage, InErrorMessage); }
	FText GetErrorMessage() const { return ErrorMessage; }

	void SetStatusMessage(const FText& InStatusMessage) { UE_MVVM_SET_PROPERTY_VALUE(StatusMessage, InStatusMessage); }
	FText GetStatusMessage() const { return StatusMessage; }

protected:
	/* These are the "Data Properties" that the ViewModel will expose to the View. */
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	int32 PlayerCurrency;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	TArray<TObjectPtr<UItemViewModel>> AvailableItems;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	TArray<TObjectPtr<UItemViewModel>> OwnedItems;


	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = SetStoreStates, Getter = GetStoreStates, Category = "Store ViewModel")
	FGameplayTagContainer StoreStates;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	FText ErrorMessage = FText::GetEmpty();

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	FText StatusMessage = FText::GetEmpty();


	/* These are the "Intent Channels" that allow the ViewModel to communicate with the Model. */
	// The UI sets this when the user hovers or selects an item. Other UI elements can bind to this to show details.
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel | Intent")
	FTransactionRequest TransactionRequest;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel | Intent")
	FString FilterText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetRefreshRequested", Getter = "GetRefreshRequested", Category = "Store ViewModel | Intent")
	bool bRefreshRequested = false;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	TObjectPtr<UItemViewModel> SelectedItem = nullptr;
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	TObjectPtr<UItemViewModel> PreviewedItem = nullptr;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	TArray<TObjectPtr<UCategoryViewModel>> SelectedCategories;


	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Transaction Request")
	ETransactionType TransactionType = ETransactionType::None;
};

