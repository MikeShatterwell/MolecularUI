// Copyright Shatterwell Inc. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>

#include "IStoreViewModelProvider.generated.h"

class UStoreViewModel;

UINTERFACE(MinimalAPI, Blueprintable)
class UStoreViewModelProvider : public UInterface
{
	GENERATED_BODY()
};

class IStoreViewModelProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MVVM")
	UStoreViewModel* GetStoreViewModel();
};