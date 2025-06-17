// Copyright Mike Desrosiers, All Rights Reserved.
#pragma once

#include <GameplayTagContainer.h>
#include <NativeGameplayTags.h>

namespace MolecularUITags
{
	namespace Store
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

