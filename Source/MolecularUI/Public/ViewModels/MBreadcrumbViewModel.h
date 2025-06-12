// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MViewModelBase.h"
#include "Subsystems/MBreadcrumbSubsystem.h"
#include "MBreadcrumbViewModel.generated.h"

/**
 * 
 */
UCLASS()
class MOLECULARUI_API UMBreadcrumbViewModel : public UMViewModelBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	TArray<FBreadcrumbEntry> Breadcrumbs;

	UFUNCTION(BlueprintCallable, Category = "Breadcrumbs")
	void SetBreadcrumbTrail(const TArray<FBreadcrumbEntry>& InTrail)
	{
		Breadcrumbs = InTrail;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Breadcrumbs);
	}
};
