// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MViewModelBase.generated.h"

/**
 * 
 */
UCLASS()
class MOLECULARUI_API UMViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void HandleButtonPress();

	UFUNCTION(BlueprintCallable)
	void HandleButtonHover();

	UFUNCTION(BlueprintCallable)
	void HandleButtonUnhover();
};
