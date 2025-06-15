// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "View/MVVMViewModelContextResolver.h"
#include "StoreViewModelResolver.generated.h"

/**
 * 
 */
UCLASS()
class MOLECULARUI_API UStoreViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
};
