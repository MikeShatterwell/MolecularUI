// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "StoreTypes.h"
#include "ItemViewModel.generated.h"

UCLASS()
class MOLECULARUI_API UItemViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void SetItemData(const FStoreItem& InItemData) { UE_MVVM_SET_PROPERTY_VALUE(ItemData, InItemData); }
	FStoreItem GetItemData() const { return ItemData; }

protected:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Item ViewModel")
	FStoreItem ItemData;
};
