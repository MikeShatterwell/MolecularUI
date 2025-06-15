// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MolecularTypes.h"
#include "ItemViewModel.generated.h"

UCLASS(Blueprintable, DisplayName = "Item ViewModel", meta = (BlueprintSpawnableComponent))
class MOLECULARUI_API UItemViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void SetItemData(const FStoreItem& InItemData) { UE_MVVM_SET_PROPERTY_VALUE(ItemData, InItemData); }
	FStoreItem GetItemData() const { return ItemData; }

	void SetInteraction(const FItemInteraction& InInteraction) { UE_MVVM_SET_PROPERTY_VALUE(Interaction, InInteraction); }
	FItemInteraction GetInteraction() const { return Interaction; }

protected:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Item ViewModel | Data")
	FStoreItem ItemData;
	
	// Stateful channel for UI interaction events such as clicks or hovers.
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Item ViewModel | Intent")
	FItemInteraction Interaction;
};
