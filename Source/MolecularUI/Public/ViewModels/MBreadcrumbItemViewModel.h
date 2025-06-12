// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/Object.h>

#include "MViewModelBase.h"
#include "Subsystems/MBreadcrumbSubsystem.h"
#include "MBreadcrumbItemViewModel.generated.h"

/**
 * 
 */
UCLASS()
class MOLECULARUI_API UMBreadcrumbItemViewModel : public UMViewModelBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify)
	FBreadcrumbEntry BreadcrumbEntry;
};
