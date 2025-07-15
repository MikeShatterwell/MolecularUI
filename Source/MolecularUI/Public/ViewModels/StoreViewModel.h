// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "CategoryViewModel.h"
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
	
	void SetTransactionRequest(const FTransactionRequest& InRequest) { UE_MVVM_SET_PROPERTY_VALUE(TransactionRequest, InRequest); }
	FTransactionRequest GetTransactionRequest() const { return TransactionRequest; }

	void AddCategory_AvailableItems(UCategoryViewModel* InCategory)
	{
		TArray<TObjectPtr<UCategoryViewModel>> NewCategories = CategoryTabs_AvailableItems;
		if (!IsValid(InCategory))
		{
			return;
		}
		const TObjectPtr<UCategoryViewModel>* ExistingCategory =
			Algo::FindByPredicate(NewCategories, [InCategory](const TObjectPtr<UCategoryViewModel>& Category)
				{ return Category->GetCategoryTag() == InCategory->GetCategoryTag(); });
		if (ExistingCategory)
		{
			return; // Category already exists, no need to add it again.
		}
		NewCategories.Add(InCategory);
		SetCategoryTabs_AvailableItems(NewCategories);
	}
	void SetCategoryTabs_AvailableItems(const TArray<UCategoryViewModel*>& InCategories) { UE_MVVM_SET_PROPERTY_VALUE(CategoryTabs_AvailableItems, InCategories); }
	const TArray<TObjectPtr<UCategoryViewModel>>& GetCategoryTabs_AvailableItems() const { return CategoryTabs_AvailableItems; }

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
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Category = "Store ViewModel")
	TArray<TObjectPtr<UCategoryViewModel>> CategoryTabs_AvailableItems;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = SetStoreStates, Getter = GetStoreStates, Category = "Store ViewModel")
	FGameplayTagContainer StoreStates;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	FText ErrorMessage = FText::GetEmpty();

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel")
	FText StatusMessage = FText::GetEmpty();
	

	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Transaction Request")
	ETransactionType TransactionType = ETransactionType::None;

	/* These are the "Intent Channels" that allow the ViewModel to communicate with the Model. */
	// The UI sets this when the user requests a transaction (e.g., purchase or sell an item) for the current TransactionType.
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel | Intent")
	FTransactionRequest TransactionRequest;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Store ViewModel | Intent")
	FString FilterText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetRefreshRequested", Getter = "GetRefreshRequested", Category = "Store ViewModel | Intent")
	bool bRefreshRequested = false;
	

};

