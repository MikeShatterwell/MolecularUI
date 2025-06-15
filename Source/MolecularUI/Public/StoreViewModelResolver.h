// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Blueprint/UserWidget.h>

#include "CoreMinimal.h"
#include "Subsystems/StoreSubsystem.h"
#include "View/MVVMViewModelContextResolver.h"
#include "ViewModels/StoreViewModel.h"
#include "StoreViewModelResolver.generated.h"

/**
 * 
 */
UCLASS()
class MOLECULARUI_API UStoreViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override
	{
		// Get StoreSubsystem
		if (const UWorld* World = UserWidget ? UserWidget->GetWorld() : nullptr)
		{
			UGameInstance* GameInstance = World->GetGameInstance();
			if (!ensure(IsValid(GameInstance)))
			{
				return nullptr;
			}
			
			UStoreSubsystem* StoreSubsystem = GameInstance->GetSubsystem<UStoreSubsystem>();
			if (!ensure(IsValid(StoreSubsystem)) || !ensure(StoreSubsystem->Implements<UStoreViewModelProvider>()))
			{
				return nullptr;
			}

			return IStoreViewModelProvider::Execute_GetStoreViewModel(StoreSubsystem);
		}
		return nullptr;
	}
};
