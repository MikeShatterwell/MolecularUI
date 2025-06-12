// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>

#include "MMockDataTypes.generated.h"

USTRUCT()
struct FMItemData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString Description;

	UPROPERTY()
	FGameplayTagContainer Tags;

	UPROPERTY()
	TSoftObjectPtr<UObject> Icon;
};