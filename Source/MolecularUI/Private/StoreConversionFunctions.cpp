// Copyright Mike Desrosiers, All Rights Reserved.

#include "StoreConversionFunctions.h"

bool UStoreConversionFunctions::Conv_AreStoreStatesEqual(const EStoreState CurrentStoreState,
	const EStoreState TestStoreState)
{
	return CurrentStoreState == TestStoreState;
}

FText UStoreConversionFunctions::Conv_StoreStateToText(const EStoreState StoreState)
{
	switch (StoreState)
	{
	case EStoreState::None: return FText::FromString("None");
	case EStoreState::Loading: return FText::FromString("Loading");
	case EStoreState::Ready: return FText::FromString("Ready");
	case EStoreState::Purchasing: return FText::FromString("Purchasing");
	case EStoreState::Error: return FText::FromString("Error");
	default: return FText::FromString("Unknown State");
	}
}
