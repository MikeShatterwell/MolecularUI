// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Models/StoreModel.h"
#include "View/MVVMViewModelContextResolver.h"
#include "StoreViewModelResolver.generated.h"

/**
 * A resolver that finds and returns the StoreViewModel instance.
 */
UCLASS()
class MOLECULARUI_API UStoreViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Molecular UI")
	TSubclassOf<class UStoreModel> StoreModelClass = UStoreModel::StaticClass();
};
