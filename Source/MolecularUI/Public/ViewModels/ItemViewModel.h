// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "InteractiveViewModelBase.h"
#include "MolecularTypes.h"
#include "ItemViewModel.generated.h"

class UCategoryViewModel;

UCLASS(Blueprintable, DisplayName = "Item ViewModel")
class MOLECULARUI_API UItemViewModel : public UInteractiveViewModelBase
{
	GENERATED_BODY()

public:
	void SetItemData(const FStoreItem& InItemData) { UE_MVVM_SET_PROPERTY_VALUE(ItemData, InItemData); }
	FStoreItem GetItemData() const { return ItemData; }

	void SetCategoryViewModels(const TArray<TObjectPtr<UCategoryViewModel>>& InCategories) { UE_MVVM_SET_PROPERTY_VALUE(CategoryViewModels, InCategories); }
	TArray<TObjectPtr<UCategoryViewModel>> GetCategoryViewModels() const { return CategoryViewModels; }

protected:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Item ViewModel | Data")
	FStoreItem ItemData;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
	TArray<TObjectPtr<UCategoryViewModel>> CategoryViewModels;
};
