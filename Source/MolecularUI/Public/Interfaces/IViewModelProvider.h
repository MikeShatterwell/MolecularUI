// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>

#include "IViewModelProvider.generated.h"

class UMVVMViewModelBase;

UINTERFACE(MinimalAPI, Blueprintable)
class UViewModelProvider : public UInterface
{
	GENERATED_BODY()
};

class IViewModelProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MVVM")
	UMVVMViewModelBase* GetViewModel(TSubclassOf<UMVVMViewModelBase> ViewModelClass);
};