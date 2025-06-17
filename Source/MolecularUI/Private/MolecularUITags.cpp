// Copyright Mike Desrosiers, All Rights Reserved.

#include "MolecularUITags.h"

namespace MolecularUITags
{
	namespace Store
	{
		UE_DEFINE_GAMEPLAY_TAG(State_None, "Store.State.None");
		UE_DEFINE_GAMEPLAY_TAG(State_Loading_Items, "Store.State.Loading.Items");
		UE_DEFINE_GAMEPLAY_TAG(State_Loading_OwnedItems, "Store.State.Loading.OwnedItems");
		UE_DEFINE_GAMEPLAY_TAG(State_Loading_Currency, "Store.State.Loading.Currency");
		UE_DEFINE_GAMEPLAY_TAG(State_Ready, "Store.State.Ready");
		UE_DEFINE_GAMEPLAY_TAG(State_Purchasing, "Store.State.Purchasing");
		UE_DEFINE_GAMEPLAY_TAG(State_Selling, "Store.State.Selling");
		UE_DEFINE_GAMEPLAY_TAG(State_Error, "Store.State.Error");
	}
}

