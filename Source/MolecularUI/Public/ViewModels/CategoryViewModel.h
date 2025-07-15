// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>

#include "InteractiveViewModelBase.h"
#include "MolecularTypes.h"
#include "CategoryViewModel.generated.h"

UCLASS(Blueprintable, DisplayName = "Category ViewModel")
class MOLECULARUI_API UCategoryViewModel : public UInteractiveViewModelBase
{
	GENERATED_BODY()
public:
	void SetUIData(const FStandardUIData& InData) { UE_MVVM_SET_PROPERTY_VALUE(UIData, InData); }
	FStandardUIData GetUIData() const { return UIData; }

	void SetCategoryTag(const FGameplayTag& InTag) { UE_MVVM_SET_PROPERTY_VALUE(CategoryTag, InTag); }
	FGameplayTag GetCategoryTag() const { return CategoryTag; }

	bool IsAll() const
	{
		return CategoryTag == MolecularUITags::Item::Category::All;
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify)
	FStandardUIData UIData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Meta = (Categories = "Item.Category"))
	FGameplayTag CategoryTag;
};
