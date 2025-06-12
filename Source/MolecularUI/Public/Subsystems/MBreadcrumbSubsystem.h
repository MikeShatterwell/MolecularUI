// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "MBreadcrumbSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FBreadcrumbEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MenuID = FName();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName = FText::GetEmpty();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBreadcrumbChanged, const TArray<FBreadcrumbEntry>&, NewTrail);

/**
 * 
 */
UCLASS()
class MOLECULARUI_API UMBreadcrumbSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Push(const FName& MenuID, FText DisplayName);

	UFUNCTION(BlueprintCallable)
	void Pop();  // Pop top
	UFUNCTION(BlueprintCallable)
	void PopToDepth(int32 Depth); // Jump back

	UFUNCTION(BlueprintCallable)
	const TArray<FBreadcrumbEntry>& GetTrail() const { return Trail; }

	/** Broadcast whenever the trail mutates */
	UPROPERTY(BlueprintAssignable)
	FOnBreadcrumbChanged OnBreadcrumbChanged;

private:
	UPROPERTY()
	TArray<FBreadcrumbEntry> Trail;
};
