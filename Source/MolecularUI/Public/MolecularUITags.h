// Copyright Mike Desrosiers, All Rights Reserved.
#pragma once

#include <GameplayTagContainer.h>
#include <NativeGameplayTags.h>

#define TAG(TagName) MOLECULARUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagName)

namespace MolecularUITags
{
	namespace Store
	{
		namespace Loading
		{
			TAG(State_Loading_Items);
			TAG(State_Loading_OwnedItems);
			TAG(State_Loading_Currency);
		}
		TAG(State_None);
		TAG(State_Ready);
		TAG(State_Purchasing);
		TAG(State_Selling);
		TAG(State_Error);
	}
}

