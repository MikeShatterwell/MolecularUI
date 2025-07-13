// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>
#include "MolecularModelSubsystem.generated.h"

class UMolecularModelBase;

/**
 * A simple subsystem that manages lifetimes of data models
 */
UCLASS(DisplayName = "Model Subsystem")
class MOLECULARUI_API UMolecularModelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// Begin USubsystem interface.
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;
	// End USubsystem interface.

	UFUNCTION(BlueprintCallable, Category="ModelSubsystem")
	UMolecularModelBase* GetModel(TSubclassOf<UMolecularModelBase> ModelClass);

	template<typename T>
	T* GetModelOfType()
	{
		return Cast<T>(GetModel(T::StaticClass()));
	}

protected:
	UPROPERTY(Transient)
	TMap<TSubclassOf<UMolecularModelBase>, TObjectPtr<UMolecularModelBase>> ModelInstances;
};
