// Copyright Mike Desrosiers, All Rights Reserved.

#include "StoreViewModelResolver.h"

#include <Blueprint/UserWidget.h>

#include "Models/StoreModel.h"
#include "Subsystems/MolecularModelSubsystem.h"
#include "ViewModels/StoreViewModel.h"

UObject* UStoreViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget,
                                                 const UMVVMView* View) const
{
	if (const UWorld* World = UserWidget ? UserWidget->GetWorld() : nullptr)
	{
		const UGameInstance* GameInstance = World->GetGameInstance();
		if (!ensure(IsValid(GameInstance)))
		{
			return nullptr;
		}

		UMolecularModelSubsystem* ModelSubsystem = GameInstance->GetSubsystem<UMolecularModelSubsystem>();
		if (!ensure(IsValid(ModelSubsystem)))
		{
			return nullptr;
		}

		UMolecularModelBase* Model = ModelSubsystem->GetModel(StoreModelClass);
		if (!ensure(IsValid(Model)))
		{
			UE_LOG(LogTemp, Error, TEXT("Model is not valid. No ViewModel will be returned."));
			return nullptr;
		}

		if (!Model->Implements<UStoreViewModelProvider>())
		{
			UE_LOG(LogTemp, Error, TEXT("Model does not implement IStoreViewModelProvider interface. No ViewModel will be returned."));
			return nullptr;
		}

		return IStoreViewModelProvider::Execute_GetStoreViewModel(Model);
	}
	return nullptr;
}
