// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/Object.h>

#include "Interfaces/IViewModelProvider.h"
#include "MolecularModelBase.generated.h"

/**
 * Generic base class for data models managed by the MolecularModelSubsystem.
 */
UCLASS(Blueprintable, EditInlineNew, Abstract)
class MOLECULARUI_API UMolecularModelBase : public UObject, public IViewModelProvider
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, Category="Model")
	void InitializeModel(UWorld* World);
	virtual void InitializeModel_Implementation(UWorld* World) {}

	UFUNCTION(BlueprintNativeEvent, Category="Model")
	void DeinitializeModel();
	virtual void DeinitializeModel_Implementation() {}

	// Begin IViewModelProvider interface.
	virtual UMVVMViewModelBase* GetViewModel_Implementation(TSubclassOf<UMVVMViewModelBase> ViewModelClass) override
	{
		return nullptr;
	}
	// End IViewModelProvider interface.
};
