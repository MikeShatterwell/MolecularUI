// Copyright Mike Desrosiers, All Rights Reserved.

#include "StoreViewModelResolver.h"

#include <Blueprint/UserWidget.h>

#include "Subsystems/StoreModelSubsystem.h"
#include "ViewModels/StoreViewModel.h"

UObject* UStoreViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget,
                                                 const UMVVMView* View) const
{
	// Get StoreSubsystem
	if (const UWorld* World = UserWidget ? UserWidget->GetWorld() : nullptr)
	{
		const UGameInstance* GameInstance = World->GetGameInstance();
		if (!ensure(IsValid(GameInstance)))
		{
			return nullptr;
		}
			
		UStoreModelSubsystem* StoreSubsystem = GameInstance->GetSubsystem<UStoreModelSubsystem>();
		if (!ensure(IsValid(StoreSubsystem)) || !ensure(StoreSubsystem->Implements<UStoreViewModelProvider>()))
		{
			return nullptr;
		}

		return IStoreViewModelProvider::Execute_GetStoreViewModel(StoreSubsystem);
	}
	return nullptr;
}
