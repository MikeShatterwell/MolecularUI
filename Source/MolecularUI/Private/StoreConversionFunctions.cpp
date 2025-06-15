// Fill out your copyright notice in the Description page of Project Settings.


#include "StoreConversionFunctions.h"

bool UStoreConversionFunctions::Conv_AreStoreStatesEqual(const EStoreState CurrentStoreState,
	const EStoreState TestStoreState)
{
	return CurrentStoreState == TestStoreState;
}

bool UStoreConversionFunctions::Conv_IsObjectValid(const UObject* Object)
{
	return IsValid(Object);
}
