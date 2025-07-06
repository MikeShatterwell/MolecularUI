// Copyright Mike Desrosiers, All Rights Reserved.

#include "MolecularUITags.h"

namespace MolecularUITags
{
	namespace Store
	{
		namespace State
		{
			namespace Loading
			{
				UE_DEFINE_GAMEPLAY_TAG(Items, "Store.State.Loading.Items");
				UE_DEFINE_GAMEPLAY_TAG(OwnedItems, "Store.State.Loading.OwnedItems");
				UE_DEFINE_GAMEPLAY_TAG(Currency, "Store.State.Loading.Currency");
			}
			UE_DEFINE_GAMEPLAY_TAG(None, "Store.State.None");
			UE_DEFINE_GAMEPLAY_TAG(Ready, "Store.State.Ready");
			UE_DEFINE_GAMEPLAY_TAG(Purchasing, "Store.State.Purchasing");
			UE_DEFINE_GAMEPLAY_TAG(Selling, "Store.State.Selling");
			UE_DEFINE_GAMEPLAY_TAG(Error, "Store.State.Error");
		}
	}
	namespace Item
	{
		namespace Category
		{
			UE_DEFINE_GAMEPLAY_TAG(Consumable, "Item.Category.Consumable");
			UE_DEFINE_GAMEPLAY_TAG(Equipment, "Item.Category.Equipment");
			UE_DEFINE_GAMEPLAY_TAG(Resource, "Item.Category.Resource");
			UE_DEFINE_GAMEPLAY_TAG(Other, "Items.Category.Other");
		}
	}
}

