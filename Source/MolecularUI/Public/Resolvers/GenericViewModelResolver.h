// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Models/StoreModel.h"
#include "View/MVVMViewModelContextResolver.h"
#include "GenericViewModelResolver.generated.h"


/**
 * A generic resolver that returns a ViewModel from a MolecularModelBase implementing IViewModelProvider.
 */
UCLASS()
class MOLECULARUI_API UGenericViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
public:
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Molecular UI")
	TObjectPtr<UMolecularModelBase> Model = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Molecular UI")
	FName ViewModelName;
};
