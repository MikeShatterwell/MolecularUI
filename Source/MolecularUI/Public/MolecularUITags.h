// Copyright Mike Desrosiers, All Rights Reserved.
#pragma once

#include <GameplayTagContainer.h>
#include <NativeGameplayTags.h>

#define TAG(TagName) MOLECULARUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagName)

namespace MolecularUITags
{
	namespace Store
	{
		namespace State
		{
			TAG(None);
			TAG(Ready);
			TAG(Purchasing);
			TAG(Selling);
			TAG(Error);
			namespace Loading
			{
				TAG(Items);
				TAG(OwnedItems);
				TAG(Currency);
			}
		}
	}
	namespace Item
	{
		namespace Category
		{
			TAG(All);
			TAG(Consumable);
			TAG(Equipment);
			TAG(Resource);
			TAG(Other);
		}
	}
	namespace InteractionSource
	{
		TAG(TabList);
		TAG(DetailsPanel);
		TAG(AvailableList);;
		TAG(OwnedList);
	}
}

