// Copyright Mike Desrosiers, All Rights Reserved.

#include "Subsystems/StoreSubsystem.h"

#include <TimerManager.h>

#include "ViewModels/StoreViewModel.h"
#include "ViewModels/ItemViewModel.h"
#include "Utils/MolecularMacros.h"
#include "Utils/LogMolecularUI.h"

void UStoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	StoreViewModel = NewObject<UStoreViewModel>(this);
	
	UE_MVVM_BIND_FIELD(StoreViewModel, FilterText, OnFilterTextChanged);
	UE_MVVM_BIND_FIELD(StoreViewModel, PurchaseRequest, OnPurchaseRequestChanged);
	
	PendingPurchaseRequests.Reserve(MaxPendingPurchaseRequestsCount);
}

void UStoreSubsystem::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	UE_MVVM_UNBIND_FIELD(StoreViewModel, FilterText);
	UE_MVVM_UNBIND_FIELD(StoreViewModel, PurchaseRequest);

	StoreViewModel = nullptr;
	for (const auto& Pair : ItemViewModelCache)
	{
		if (IsValid(Pair.Value))
		{
			// Unbind any field notifications from the ItemViewModel.
			UE_MVVM_UNBIND_FIELD(Pair.Value, Interaction);
		}
	}
	ItemViewModelCache.Empty();

	BackendStoreItems.Empty();
	BackendOwnedStoreItems.Empty();
	BackendPlayerCurrency = INDEX_NONE;

	Super::Deinitialize();
}

UStoreViewModel* UStoreSubsystem::GetStoreViewModel_Implementation()
{
	if (!IsValid(StoreViewModel))
	{
		UE_LOG(LogMolecularUI, Error, TEXT("[%hs] UStoreSubsystem is not initialized yet!"), __FUNCTION__);
		return nullptr;
	}

	if (StoreViewModel->GetStoreState() == EStoreState::None)
	{
		StoreViewModel->SetStoreState(EStoreState::Loading);
		// Load stuff on initial open
		LazyLoadStoreItems();
		LazyLoadStoreCurrency();
		LazyLoadOwnedItems(); 
	}

	return StoreViewModel;
}

void UStoreSubsystem::OnFilterTextChanged(UObject* Object, UE::FieldNotification::FFieldId Field)
{
	FilterAvailableStoreItems(StoreViewModel->GetFilterText());
}

void UStoreSubsystem::OnPurchaseRequestChanged(UObject* Object, UE::FieldNotification::FFieldId Field)
{
	const FPurchaseRequest& PurchaseRequestId = StoreViewModel->GetPurchaseRequest();

	// Only process valid requests
	if (!PurchaseRequestId.IsValid())
	{
		return;
	}

	// If the store isn't ready, queue or reset the request and exit early.
	if (StoreViewModel->GetStoreState() != EStoreState::Ready)
	{
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Store not ready. Queuing purchase request for %s"), __FUNCTION__, *PurchaseRequestId.ItemId.ToString());

		if (PendingPurchaseRequests.Num() < MaxPendingPurchaseRequestsCount)
		{
			PendingPurchaseRequests.Add(PurchaseRequestId);
		}
		else
		{
			UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Pending purchase queue full. Discarding request for %s"), __FUNCTION__, *PurchaseRequestId.ItemId.ToString());
		}

		StoreViewModel->SetPurchaseRequest(FPurchaseRequest());
		return;
	}

	LazyPurchaseItem(PurchaseRequestId);
}

void UStoreSubsystem::OnItemInteractionChanged(UObject* Object, UE::FieldNotification::FFieldId Field)
{
	UItemViewModel* ItemVM = Cast<UItemViewModel>(Object);
	if (!ensure(IsValid(ItemVM)))
	{
		return;
	}

	const FItemInteraction& Interaction = ItemVM->GetInteraction();
	if (!Interaction.IsValid())
	{
		return; // No valid interaction to process
	}

	// Handle the interaction based on its type
	switch (Interaction.Type)
	{
	case EItemInteractionType::Hovered:
		if (GEngine)
		{
			const FString& ItemName = ItemVM->GetItemData().UIData.DisplayName.ToString();
			GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Item Hovered: %s"), *ItemName));
		}
		break;
	case EItemInteractionType::Unhovered:
		if (GEngine)
		{
			const FString& ItemName = ItemVM->GetItemData().UIData.DisplayName.ToString();
			GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Yellow, FString::Printf(TEXT("Item UnHovered: %s"), *ItemName));
		}
		break;
	case EItemInteractionType::Clicked:
		if (GEngine)
		{
			const FString& ItemName = ItemVM->GetItemData().UIData.DisplayName.ToString();
			GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Yellow, FString::Printf(TEXT("Item Clicked: %s"), *ItemName));
			StoreViewModel->SetSelectedItem(ItemVM);
		}
	default:
		break;
	}
}

void UStoreSubsystem::LazyLoadStoreItems()
{
	// 1. Immediately set the UI to a loading state.
	StoreViewModel->SetStoreState(EStoreState::Loading);

	// 2. Define what should happen on success.
	auto OnSuccess = [this]
	{
		CreateDummyStoreData();

		TArray<TObjectPtr<UItemViewModel>> FilteredItems;
		FilteredItems.Reserve(BackendStoreItems.Num());
		const FString& CurrentFilter = StoreViewModel->GetFilterText();

		for (const FStoreItem& ItemData : BackendStoreItems)
		{
			if (CurrentFilter.IsEmpty() || ItemData.UIData.DisplayName.ToString().Contains(CurrentFilter))
			{
				UItemViewModel* ItemVM = GetOrCreateItemViewModel(ItemData);
				FilteredItems.Add(ItemVM);
			}
		}

		StoreViewModel->SetAvailableItems(FilteredItems);
		StoreViewModel->SetStoreState(EStoreState::Ready); // Set to ready ONLY after success.
		ProcessPendingPurchaseRequests();
	};

	// 3. Define what should happen on failure.
	auto OnFailure = [this]
	{
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Mock failure loading store items."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(FText::FromString("There was a problem loading items from the store. Please try again later."));
		StoreViewModel->SetStoreState(EStoreState::Error);
	};

	// 4. Call the macro with the callbacks and a desired failure chance
	constexpr float FailureChance = 0.15f;
	FETCH_MOCK_DATA_WITH_RESULT(ItemLoadTimerHandle, OnSuccess, OnFailure, FailureChance);
}

void UStoreSubsystem::LazyLoadOwnedItems()
{
	StoreViewModel->SetStoreState(EStoreState::Loading);

	auto OnSuccess = [this]
	{
		if (BackendOwnedStoreItems.IsEmpty())
		{
			CreateDummyOwnedStoreData();
		}

		TArray<TObjectPtr<UItemViewModel>> OwnedItemVMs;
		OwnedItemVMs.Reserve(BackendOwnedStoreItems.Num());

		for (const FStoreItem& OwnedItemData : BackendOwnedStoreItems)
		{
			UItemViewModel* ItemVM = GetOrCreateItemViewModel(OwnedItemData);
			OwnedItemVMs.Add(ItemVM);
		}

		StoreViewModel->SetOwnedItems(OwnedItemVMs);
	};

	auto OnFailure = [this]
	{
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Mock failure loading owned items."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(FText::FromString("There was a problem loading owned items. Please try again later."));
	};

	constexpr float FailureChance = 0.15f; 
	FETCH_MOCK_DATA_WITH_RESULT(OwnedItemLoadTimerHandle, OnSuccess, OnFailure, FailureChance);
}

void UStoreSubsystem::LazyLoadStoreCurrency()
{
	StoreViewModel->SetStoreState(EStoreState::Loading);

	auto OnSuccess = [this]()
	{
		if (BackendPlayerCurrency < 0)
		{
			CreateDummyPlayerCurrency();
		}

		const int32 LoadedCurrency = BackendPlayerCurrency;
		StoreViewModel->SetPlayerCurrency(LoadedCurrency);

		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Player currency loaded: %d"), __FUNCTION__, LoadedCurrency);
		StoreViewModel->SetStoreState(EStoreState::Ready);
	};

	auto OnFailure = [this]()
	{
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Mock failure loading store currency."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(FText::FromString("There was a problem loading player currency. Please try again later."));
		StoreViewModel->SetStoreState(EStoreState::Error);
	};

	constexpr float FailureChance = 0.15f;
	FETCH_MOCK_DATA_WITH_RESULT(CurrencyLoadTimerHandle, OnSuccess, OnFailure, FailureChance);
}

void UStoreSubsystem::LazyPurchaseItem(const FPurchaseRequest& PurchaseRequest)
{
	StoreViewModel->SetStoreState(EStoreState::Purchasing);

	// Define what should happen on success.
	auto OnSuccess = [this, PurchaseRequest]
	{
		const FStoreItem* FoundItem = BackendStoreItems.FindByPredicate([&](const FStoreItem& Item)
		{
			return Item.ItemId == PurchaseRequest.ItemId;
		});

		if (FoundItem == nullptr)
		{
			StoreViewModel->SetStoreState(EStoreState::Error);
			StoreViewModel->SetErrorMessage(FText::FromString("Item not found."));
			return;
		}

		if (BackendPlayerCurrency < FoundItem->Cost)
		{
			StoreViewModel->SetStoreState(EStoreState::Error);
			StoreViewModel->SetErrorMessage(FText::FromString("Not enough currency."));
			return;
		}

		// Simulate a successful purchase
		const FStoreItem PurchasedItem = *FoundItem; // Make a copy to avoid reference issues after removal
		BackendPlayerCurrency -= PurchasedItem.Cost;
		BackendOwnedStoreItems.Add(PurchasedItem);
		BackendStoreItems.Remove(PurchasedItem);
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Purchased item: %s for %d currency."), __FUNCTION__, *PurchasedItem.ItemId.ToString(), PurchasedItem.Cost);

		StoreViewModel->SetPlayerCurrency(BackendPlayerCurrency);
		StoreViewModel->SetPurchaseRequest(FPurchaseRequest()); // Clear the request

		LazyLoadStoreItems(); // Refresh available items
		LazyLoadOwnedItems(); // Refresh owned items

		StoreViewModel->SetStoreState(EStoreState::Ready);
		ProcessPendingPurchaseRequests();
	};

	// Define what should happen on failure.
	auto OnFailure = [this]
	{
		StoreViewModel->SetStoreState(EStoreState::Error);
		StoreViewModel->SetErrorMessage(FText::FromString("Purchase failed. Please try again later."));
	};

	// Simulate purchase with some failure chance.
	constexpr float FailureChance = 0.15f;
	FETCH_MOCK_DATA_WITH_RESULT(ItemPurchaseTimerHandle, OnSuccess, OnFailure, FailureChance);
}

void UStoreSubsystem::ProcessPendingPurchaseRequests()
{
	if (StoreViewModel->GetStoreState() != EStoreState::Ready || PendingPurchaseRequests.Num() == 0)
	{
		return;
	}

	const FPurchaseRequest& NextRequest = PendingPurchaseRequests[0];
	PendingPurchaseRequests.RemoveAt(0);

	// Trigger processing by setting the ViewModel property, which will call OnPurchaseRequestChanged again.
	StoreViewModel->SetPurchaseRequest(NextRequest);
}

void UStoreSubsystem::FilterAvailableStoreItems(const FString& FilterText)
{
	TArray<TObjectPtr<UItemViewModel>> FilteredItems;
	FilteredItems.Reserve(BackendStoreItems.Num());

	for (const FStoreItem& ItemData : BackendStoreItems)
	{
		if (FilterText.IsEmpty() || ItemData.UIData.DisplayName.ToString().Contains(FilterText))
		{
			UItemViewModel* ItemVM = GetOrCreateItemViewModel(ItemData);
			FilteredItems.Add(ItemVM);
		}
	}

	StoreViewModel->SetAvailableItems(FilteredItems);
}

void UStoreSubsystem::CreateDummyStoreData()
{
	BackendStoreItems.Empty(128);
	// Initialize with some dummy data for the demo.
	// This is not good practice for production code
	// In a real application, this data would come from a database or a backend API
	// Or a more sensible test data architecture.
	{
		BackendStoreItems.Add(FStoreItem{FName{"HealthPotion"},		 50,  FItemUIData{INVTEXT("Health Potion")}});
		BackendStoreItems.Add(FStoreItem{FName{"ManaPotion"},		   30,  FItemUIData{INVTEXT("Mana Potion")}});
		BackendStoreItems.Add(FStoreItem{FName{"Sword"},			   100,  FItemUIData{INVTEXT("Sword")}});
		BackendStoreItems.Add(FStoreItem{FName{"Shield"},			  75,   FItemUIData{INVTEXT("Shield")}});
		BackendStoreItems.Add(FStoreItem{FName{"Bow"},				 120,  FItemUIData{INVTEXT("Bow")}});
		BackendStoreItems.Add(FStoreItem{FName{"ArrowPack"},		   20,   FItemUIData{INVTEXT("Arrow Pack")}});
		BackendStoreItems.Add(FStoreItem{FName{"Helmet"},			  60,   FItemUIData{INVTEXT("Helmet")}});
		BackendStoreItems.Add(FStoreItem{FName{"Armor"},			   150,  FItemUIData{INVTEXT("Armor")}});
		BackendStoreItems.Add(FStoreItem{FName{"Boots"},			   40,   FItemUIData{INVTEXT("Boots")}});
		BackendStoreItems.Add(FStoreItem{FName{"Ring"},				80,   FItemUIData{INVTEXT("Ring")}});
		BackendStoreItems.Add(FStoreItem{FName{"Amulet"},			  90,   FItemUIData{INVTEXT("Amulet")}});
		BackendStoreItems.Add(FStoreItem{FName{"PotionOfStrength"},	200,  FItemUIData{INVTEXT("Potion of Strength")}});
		BackendStoreItems.Add(FStoreItem{FName{"PotionOfAgility"},	 150,  FItemUIData{INVTEXT("Potion of Agility")}});
		BackendStoreItems.Add(FStoreItem{FName{"PotionOfIntelligence"},180,  FItemUIData{INVTEXT("Potion of Intelligence")}});
		BackendStoreItems.Add(FStoreItem{FName{"ScrollOfFireball"},	250,  FItemUIData{INVTEXT("Scroll of Fireball")}});
		BackendStoreItems.Add(FStoreItem{FName{"ScrollOfTeleportation"},300, FItemUIData{INVTEXT("Scroll of Teleportation")}});
		BackendStoreItems.Add(FStoreItem{FName{"ElixirOfLife"},		500,  FItemUIData{INVTEXT("Elixir of Life")}});
		BackendStoreItems.Add(FStoreItem{FName{"MagicWand"},		   400,  FItemUIData{INVTEXT("Magic Wand")}});
		BackendStoreItems.Add(FStoreItem{FName{"DragonScale"},		 600,  FItemUIData{INVTEXT("Dragon Scale")}});
		BackendStoreItems.Add(FStoreItem{FName{"PhoenixFeather"},	  700,  FItemUIData{INVTEXT("Phoenix Feather")}});
		BackendStoreItems.Add(FStoreItem{FName{"CrystalOfWisdom"},	 800,  FItemUIData{INVTEXT("Crystal of Wisdom")}});
		BackendStoreItems.Add(FStoreItem{FName{"GoldenApple"},		 900,  FItemUIData{INVTEXT("Golden Apple")}});
		BackendStoreItems.Add(FStoreItem{FName{"SilverCoin"},		  10,   FItemUIData{INVTEXT("Silver Coin")}});
		BackendStoreItems.Add(FStoreItem{FName{"BronzeCoin"},		  5,	FItemUIData{INVTEXT("Bronze Coin")}});
		BackendStoreItems.Add(FStoreItem{FName{"MysticGem"},		   250,  FItemUIData{INVTEXT("Mystic Gem")}});
		BackendStoreItems.Add(FStoreItem{FName{"EnchantedBook"},	   300,  FItemUIData{INVTEXT("Enchanted Book")}});
		BackendStoreItems.Add(FStoreItem{FName{"AncientArtifact"},	 1000, FItemUIData{INVTEXT("Ancient Artifact")}});
		BackendStoreItems.Add(FStoreItem{FName{"HealingHerb"},		 20,   FItemUIData{INVTEXT("Healing Herb")}});
		BackendStoreItems.Add(FStoreItem{FName{"ManaHerb"},			15,   FItemUIData{INVTEXT("Mana Herb")}});
		BackendStoreItems.Add(FStoreItem{FName{"StaminaPotion"},	   40,   FItemUIData{INVTEXT("Stamina Potion")}});
		BackendStoreItems.Add(FStoreItem{FName{"SpeedBoots"},		  70,   FItemUIData{INVTEXT("Speed Boots")}});
		BackendStoreItems.Add(FStoreItem{FName{"StealthCloak"},		120,  FItemUIData{INVTEXT("Stealth Cloak")}});
		BackendStoreItems.Add(FStoreItem{FName{"BattleAxe"},		   200,  FItemUIData{INVTEXT("Battle Axe")}});
		BackendStoreItems.Add(FStoreItem{FName{"Crossbow"},			180,  FItemUIData{INVTEXT("Crossbow")}});
		BackendStoreItems.Add(FStoreItem{FName{"MagicStaff"},		  350,  FItemUIData{INVTEXT("Magic Staff")}});
		BackendStoreItems.Add(FStoreItem{FName{"HealingScroll"},	   220,  FItemUIData{INVTEXT("Healing Scroll")}});
		BackendStoreItems.Add(FStoreItem{FName{"FireBomb"},			150,  FItemUIData{INVTEXT("Fire Bomb")}});
		BackendStoreItems.Add(FStoreItem{FName{"IceBomb"},			 160,  FItemUIData{INVTEXT("Ice Bomb")}});
		BackendStoreItems.Add(FStoreItem{FName{"LightningBomb"},	   170,  FItemUIData{INVTEXT("Lightning Bomb")}});
		BackendStoreItems.Add(FStoreItem{FName{"PoisonBomb"},		  140,  FItemUIData{INVTEXT("Poison Bomb")}});
		BackendStoreItems.Add(FStoreItem{FName{"TeleportationStone"},  300,  FItemUIData{INVTEXT("Teleportation Stone")}});
		BackendStoreItems.Add(FStoreItem{FName{"MysticOrb"},		   400,  FItemUIData{INVTEXT("Mystic Orb")}});
		BackendStoreItems.Add(FStoreItem{FName{"AncientScroll"},	   500,  FItemUIData{INVTEXT("Ancient Scroll")}});
		BackendStoreItems.Add(FStoreItem{FName{"DragonEgg"},		   1000, FItemUIData{INVTEXT("Dragon Egg")}});
		BackendStoreItems.Add(FStoreItem{FName{"PhoenixEgg"},		  1200, FItemUIData{INVTEXT("Phoenix Egg")}});
		BackendStoreItems.Add(FStoreItem{FName{"CrystalBall"},		 600,  FItemUIData{INVTEXT("Crystal Ball")}});
		BackendStoreItems.Add(FStoreItem{FName{"ElvenBow"},			250,  FItemUIData{INVTEXT("Elven Bow")}});
		BackendStoreItems.Add(FStoreItem{FName{"DwarvenHammer"},	   300,  FItemUIData{INVTEXT("Dwarven Hammer")}});
		BackendStoreItems.Add(FStoreItem{FName{"HolyGrail"},		   800,  FItemUIData{INVTEXT("Holy Grail")}});
		BackendStoreItems.Add(FStoreItem{FName{"CursedDagger"},		400,  FItemUIData{INVTEXT("Cursed Dagger")}});
		BackendStoreItems.Add(FStoreItem{FName{"VampireFang"},		 450,  FItemUIData{INVTEXT("Vampire Fang")}});
		BackendStoreItems.Add(FStoreItem{FName{"WerewolfClaw"},		500,  FItemUIData{INVTEXT("Werewolf Claw")}});
	}
}

void UStoreSubsystem::CreateDummyOwnedStoreData()
{
	BackendOwnedStoreItems.Empty(32);;
	BackendOwnedStoreItems.Add(FStoreItem{FName{"LameSword"}, 5, FItemUIData{INVTEXT("Lame Sword")}});
	BackendOwnedStoreItems.Add(FStoreItem{FName{"BasicShield"}, 10, FItemUIData{INVTEXT("Basic Shield")}});
	BackendOwnedStoreItems.Add(FStoreItem{FName{"OldHelmet"}, 15, FItemUIData{INVTEXT("Old Helmet")}});
	BackendOwnedStoreItems.Add(FStoreItem{FName{"RustyBoots"}, 8, FItemUIData{INVTEXT("Rusty Boots")}});
	BackendOwnedStoreItems.Add(FStoreItem{FName{"WoodenBow"}, 12, FItemUIData{INVTEXT("Wooden Bow")}});
}

void UStoreSubsystem::CreateDummyPlayerCurrency()
{
	// Simulate player currency
	BackendPlayerCurrency = 12124;
}

UItemViewModel* UStoreSubsystem::GetOrCreateItemViewModel(const FStoreItem& ItemData)
{
	// Check if the ViewModel already exists in the cache and is valid.
	if (TObjectPtr<UItemViewModel>* FoundViewModel = ItemViewModelCache.Find(ItemData.ItemId))
	{
		if (IsValid(*FoundViewModel))
		{
			// It exists, update its data just in case it changed and return it.
			(*FoundViewModel)->SetItemData(ItemData);
			return *FoundViewModel;
		}
	}

	// If not found or was invalid (e.g. garbage collected), create a new one.
	UItemViewModel* NewItemVM = NewObject<UItemViewModel>(StoreViewModel);
	NewItemVM->SetItemData(ItemData);

	// Bind the interaction FieldNotify to the ViewModel's OnItemInteractionChanged handler.
	UE_MVVM_BIND_FIELD(NewItemVM, Interaction, OnItemInteractionChanged);

	// Add the new ViewModel to the cache for future reuse.
	ItemViewModelCache.Add(ItemData.ItemId, NewItemVM);

	return NewItemVM;
}