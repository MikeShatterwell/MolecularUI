// Copyright Mike Desrosiers, All Rights Reserved.

#include "StoreConversionFunctions.h"
#include "MolecularUITags.h"

bool UStoreConversionFunctions::Conv_HasStoreState(const FGameplayTagContainer CurrentStoreStates,
        const FGameplayTag TestStoreState)
{
        return CurrentStoreStates.HasTagExact(TestStoreState);
}

FText UStoreConversionFunctions::Conv_StoreStateToText(const FGameplayTag StoreState)
{
        if (StoreState == MolecularUI::Tags::State_None) return FText::FromString("None");
        if (StoreState == MolecularUI::Tags::State_Loading_Items) return FText::FromString("Loading Items");
        if (StoreState == MolecularUI::Tags::State_Loading_OwnedItems) return FText::FromString("Loading Owned Items");
        if (StoreState == MolecularUI::Tags::State_Loading_Currency) return FText::FromString("Loading Currency");
        if (StoreState == MolecularUI::Tags::State_Ready) return FText::FromString("Ready");
        if (StoreState == MolecularUI::Tags::State_Purchasing) return FText::FromString("Purchasing");
        if (StoreState == MolecularUI::Tags::State_Error) return FText::FromString("Error");
        return FText::FromString("Unknown State");
}
