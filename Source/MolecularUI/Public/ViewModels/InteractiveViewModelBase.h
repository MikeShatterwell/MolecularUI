// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MolecularTypes.h"
#include "InteractiveViewModelBase.generated.h"

/**
 * Base class for ViewModels that handle user interactions using stateful communication.
 */
UCLASS(BlueprintType)
class MOLECULARUI_API UInteractiveViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Item ViewModel | Intent")
	void SetInteraction(const EStatefulInteraction InType, const FName InSource = NAME_None)
	{
		const FInteractionState NewInteraction(InType, InSource);
		UE_MVVM_SET_PROPERTY_VALUE(Interaction, NewInteraction);
	}

	const FInteractionState& GetInteraction() const { return Interaction; }

protected:
	// Stateful channel for UI interaction events such as clicks or hovers.
	// Bind to SetInteraction to update this state from an interaction event.
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	FInteractionState Interaction;
};
