// Copyright Mike Desrosiers, All Rights Reserved.

#include "Resolvers/GenericViewModelResolver.h"

#include <MVVMViewModelBase.h>
#include <Blueprint/UserWidget.h>

#include "Subsystems/MolecularModelSubsystem.h"
#include "Utils/LogMolecularUI.h"

UObject* UGenericViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget,
												   const UMVVMView* View) const
{
	if (ModelClass == nullptr)
	{
		UE_LOG(LogMolecularUI, Error, TEXT("Invalid ModelClass for generic view model resolver looking for ViewModel %s"),
			   *ViewModelName.ToString());
		return nullptr;
	}

	UWorld* World = UserWidget ? UserWidget->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		UE_LOG(LogMolecularUI, Error, TEXT("UserWidget does not have a valid world for generic view model resolver"));
		return nullptr;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	if (!IsValid(GameInstance))
	{
		UE_LOG(LogMolecularUI, Error, TEXT("World does not have a valid game instance for generic view model resolver"));
		return nullptr;
	}

	UMolecularModelSubsystem* ModelSubsystem = GameInstance->GetSubsystem<UMolecularModelSubsystem>();
	if (!IsValid(ModelSubsystem))
	{
		UE_LOG(LogMolecularUI, Error, TEXT("Game instance does not have a valid MolecularModelSubsystem for generic view model resolver"));
		return nullptr;
	}

	UMolecularModelBase* ModelInstance = ModelSubsystem->GetModel(ModelClass);
	if (!IsValid(ModelInstance))
	{
		UE_LOG(LogMolecularUI, Error, TEXT("Failed to get model instance of type %s for generic view model resolver"),
			   *ModelClass->GetName());
		return nullptr;
	}

	// Check if the model implements the generic provider interface.
	if (!ModelInstance->Implements<UViewModelProvider>())
	{
		UE_LOG(LogTemp, Error, TEXT("Model does not implement IViewModelProvider"));
		return nullptr;
	}

	const TSubclassOf<UMVVMViewModelBase> ExpectedTypeClass = ExpectedType->GetDefaultObject<UMVVMViewModelBase>()->GetClass();
	const FMVVMViewModelContext ViewModelContext(ExpectedTypeClass, ViewModelName);
	UMVVMViewModelBase* ViewModel = IViewModelProvider::Execute_GetViewModel(ModelInstance, ViewModelContext);
	if (!IsValid(ViewModel))
	{
		UE_LOG(LogTemp, Error, TEXT("IViewModelProvider returned null"));
		return nullptr;
	}

	if (!ViewModel->IsA(ExpectedType))
	{
		UE_LOG(LogTemp, Error, TEXT("Returned ViewModel is not of expected type"));
		return nullptr;
	}

	return ViewModel;
}
