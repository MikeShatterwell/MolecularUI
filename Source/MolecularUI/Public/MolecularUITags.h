#pragma once

#include <GameplayTagContainer.h>
#include "MolecularUITags.generated.h"

/** Collection of gameplay tags used by MolecularUI */
USTRUCT()
struct FMolecularUITags
{
    GENERATED_BODY()
};

namespace MolecularUI
{
namespace Tags
{
    // Store state tags
    MOLECULARUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_None);
    MOLECULARUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Loading_Items);
    MOLECULARUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Loading_OwnedItems);
    MOLECULARUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Loading_Currency);
    MOLECULARUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Ready);
    MOLECULARUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Purchasing);
    MOLECULARUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Error);
}
}

