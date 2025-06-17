// Copyright Mike Desrosiers, All Rights Reserved.

#include "Subsystems/StoreSubsystem.h"

#include <TimerManager.h>

#include "ViewModels/StoreViewModel.h"
#include "ViewModels/ItemViewModel.h"
#include "Utils/MolecularMacros.h"
#include "MolecularUITags.h"
#include "Utils/LogMolecularUI.h"

namespace
{
	/** RAII helper that tracks a store state for the lifetime of an async operation. */
	struct FScopedStoreState
	{
		TWeakObjectPtr<UStoreViewModel> ViewModel;
		FGameplayTag State;

		FScopedStoreState(UStoreViewModel* InViewModel, const FGameplayTag& InState)
			: ViewModel(InViewModel), State(InState)
		{
			if (ViewModel.IsValid())
			{
				ViewModel->AddStoreState(State);
			}
		}

		~FScopedStoreState()
		{
			if (ViewModel.IsValid())
			{
				ViewModel->RemoveStoreState(State);
			}
		}
	};
}

void UStoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	StoreViewModel = NewObject<UStoreViewModel>(this);

	// Start in a "None" state so initial loads can be triggered on first access.
	StoreViewModel->AddStoreState(MolecularUITags::Store::State_None);

	UE_MVVM_BIND_FIELD(StoreViewModel, FilterText, OnFilterTextChanged);
	UE_MVVM_BIND_FIELD(StoreViewModel, TransactionRequest, OnTransactionRequestChanged);
}

void UStoreSubsystem::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	UE_MVVM_UNBIND_FIELD(StoreViewModel, FilterText);
	UE_MVVM_UNBIND_FIELD(StoreViewModel, TransactionRequest);

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

	if (StoreViewModel->HasStoreState(MolecularUITags::Store::State_None))
	{
		StoreViewModel->RemoveStoreState(MolecularUITags::Store::State_None);

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

void UStoreSubsystem::OnTransactionRequestChanged(UObject* Object, UE::FieldNotification::FFieldId Field)
{
	const FTransactionRequest& TransactionRequest = StoreViewModel->GetTransactionRequest();

	// Only process valid requests
	if (!TransactionRequest.IsValid())
	{
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Invalid transaction request. Ignoring."), __FUNCTION__);
		StoreViewModel->SetTransactionRequest(FTransactionRequest());
		StoreViewModel->SetErrorMessage(FText::FromString("Invalid transaction request."));
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
		return;
	}

	// If the store isn't ready, queue or reset the request and exit early.
	if (!StoreViewModel->HasStoreState(MolecularUITags::Store::State_Ready))
	{
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Store not ready. Failing transaction request for %s"), __FUNCTION__,
		       *TransactionRequest.ToString());

		StoreViewModel->SetTransactionRequest(FTransactionRequest());
		StoreViewModel->SetErrorMessage(FText::FromString("Store not ready. Please try again later."));
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
		return;
	}

	switch (StoreViewModel->GetTransactionType())
	{
	case ETransactionType::None:
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] No transaction type set for request: %s"), __FUNCTION__,
		       *TransactionRequest.ToString());
		break;
	case ETransactionType::Purchase:
		LazyPurchaseItem(TransactionRequest);
		break;
	case ETransactionType::Sell:
		ensure(false); // Not implemented yet
		break;
	}
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
			GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Yellow,
			                                 FString::Printf(TEXT("Item Hovered: %s"), *ItemName));
		}
		break;
	case EItemInteractionType::Unhovered:
		if (GEngine)
		{
			const FString& ItemName = ItemVM->GetItemData().UIData.DisplayName.ToString();
			GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Yellow,
			                                 FString::Printf(TEXT("Item UnHovered: %s"), *ItemName));
		}
		break;
	case EItemInteractionType::Clicked:
		if (GEngine)
		{
			const FString& ItemName = ItemVM->GetItemData().UIData.DisplayName.ToString();
			GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Yellow,
			                                 FString::Printf(TEXT("Item Clicked: %s"), *ItemName));
			StoreViewModel->SetSelectedItem(ItemVM);
			if (BackendOwnedStoreItems.Contains(ItemVM->GetItemData()))
			{
				StoreViewModel->SetTransactionType(ETransactionType::Sell);
			} else if (BackendStoreItems.Contains(ItemVM->GetItemData()))
			{
				StoreViewModel->SetTransactionType(ETransactionType::Purchase);
			} else
			{
				StoreViewModel->SetTransactionType(ETransactionType::None);
			}
		}
		break;
	default:
		break;
	}
}

void UStoreSubsystem::LazyLoadStoreItems()
{
	TSharedRef<FScopedStoreState> LoadingScope = MakeShared<FScopedStoreState>(
		StoreViewModel, MolecularUITags::Store::State_Loading_Items);

	// 2. Define what should happen on success.
	auto OnSuccess = [this, LoadingScope]
	{
		CreateDummyStoreData();

		TArray<TObjectPtr<UItemViewModel>> StoreItems;
		StoreItems.Reserve(BackendStoreItems.Num());

		for (const FStoreItem& ItemData : BackendStoreItems)
		{
			// Avoid adding items that are already owned.
			if (BackendOwnedStoreItems.Contains(ItemData))
			{
				continue;
			}
			UItemViewModel* ItemVM = GetOrCreateItemViewModel(ItemData);
			StoreItems.AddUnique(ItemVM);
		}

		StoreViewModel->SetAvailableItems(StoreItems);
	};

	// 3. Define what should happen on failure.
	auto OnFailure = [this, LoadingScope]
	{
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Mock failure loading store items."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(
			FText::FromString("There was a problem loading items from the store. Please try again later."));
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
	};

	// 4. Call the macro with the callbacks and a desired failure chance
	constexpr float FailureChance = 0.15f;
	constexpr float MinDelay = 3.5f;
	constexpr float MaxDelay = 8.5f;
	FETCH_MOCK_DATA(ItemLoadTimerHandle, OnSuccess, OnFailure, FailureChance, MinDelay, MaxDelay);
}

void UStoreSubsystem::LazyLoadOwnedItems()
{
	TSharedRef<FScopedStoreState> LoadingScope = MakeShared<FScopedStoreState>(
		StoreViewModel, MolecularUITags::Store::State_Loading_OwnedItems);

	auto OnSuccess = [this, LoadingScope]
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
			OwnedItemVMs.AddUnique(ItemVM);
		}

		StoreViewModel->SetOwnedItems(OwnedItemVMs);
	};

	auto OnFailure = [this, LoadingScope]
	{
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Mock failure loading owned items."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(
			FText::FromString("There was a problem loading owned items. Please try again later."));
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
	};

	FETCH_MOCK_DATA(OwnedItemLoadTimerHandle, OnSuccess, OnFailure);
}

void UStoreSubsystem::LazyLoadStoreCurrency()
{
	TSharedRef<FScopedStoreState> LoadingScope = MakeShared<FScopedStoreState>(
		StoreViewModel, MolecularUITags::Store::State_Loading_Currency);

	auto OnSuccess = [this, LoadingScope]()
	{
		if (BackendPlayerCurrency < 0)
		{
			CreateDummyPlayerCurrency();
		}

		const int32 LoadedCurrency = BackendPlayerCurrency;
		StoreViewModel->SetPlayerCurrency(LoadedCurrency);

		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Player currency loaded: %d"), __FUNCTION__, LoadedCurrency);
	};

	auto OnFailure = [this, LoadingScope]()
	{
		UE_LOG(LogMolecularUI, Warning, TEXT("[%hs] Mock failure loading store currency."), __FUNCTION__);
		StoreViewModel->SetErrorMessage(
			FText::FromString("There was a problem loading player currency. Please try again later."));
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
	};

	FETCH_MOCK_DATA(CurrencyLoadTimerHandle, OnSuccess, OnFailure);
}

void UStoreSubsystem::LazyPurchaseItem(const FTransactionRequest& PurchaseRequest)
{
	TSharedRef<FScopedStoreState> PurchaseScope = MakeShared<FScopedStoreState>(
		StoreViewModel, MolecularUITags::Store::State_Purchasing);

	// Define what should happen on success.
	auto OnSuccess = [this, PurchaseRequest, PurchaseScope]
	{
		const FStoreItem* FoundItem = BackendStoreItems.FindByPredicate([&](const FStoreItem& Item)
		{
			return Item.ItemId == PurchaseRequest.ItemId;
		});

		// Check if the item was found
		if (FoundItem == nullptr)
		{
			StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
			StoreViewModel->SetErrorMessage(
				FText::FromString("Item not found. It could have been removed from the store's available items."));
			return;
		}

		// Check if the player has enough currency to purchase the item
		if (BackendPlayerCurrency < FoundItem->Cost)
		{
			StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
			StoreViewModel->SetErrorMessage(FText::FromString("Not enough currency."));
			return;
		}

		// Check if the item is already owned
		if (BackendOwnedStoreItems.Contains(*FoundItem))
		{
			StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
			StoreViewModel->SetErrorMessage(FText::FromString("Item already owned."));
			return;
		}

		// Simulate a successful purchase
		const FStoreItem PurchasedItem = *FoundItem; // Make a copy to avoid reference issues after removal
		BackendPlayerCurrency -= PurchasedItem.Cost;
		BackendOwnedStoreItems.Add(PurchasedItem);
		BackendStoreItems.Remove(PurchasedItem);
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Purchased item: %s for %d currency."), __FUNCTION__,
		       *PurchasedItem.ItemId.ToString(), PurchasedItem.Cost);

		StoreViewModel->SetPlayerCurrency(BackendPlayerCurrency);
		StoreViewModel->SetTransactionRequest(FTransactionRequest()); // Clear the request
		StoreViewModel->SetTransactionType(ETransactionType::None); // Reset transaction type
		StoreViewModel->SetSelectedItem(nullptr); // Clear the selected item

		LazyLoadStoreItems(); // Refresh available items
		LazyLoadOwnedItems(); // Refresh owned items
	};

	// Define what should happen on failure.
	auto OnFailure = [this, PurchaseScope]
	{
		StoreViewModel->AddStoreState(MolecularUITags::Store::State_Error);
		StoreViewModel->SetErrorMessage(FText::FromString("Purchase failed. Please try again later."));
	};

	// Simulate purchase with some failure chance.
	FETCH_MOCK_DATA(ItemPurchaseTimerHandle, OnSuccess, OnFailure);
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
		BackendStoreItems.Add(FStoreItem{FName{"HealthPotion"}, 50, false, FItemUIData{INVTEXT("Health Potion")}});
		BackendStoreItems.Add(FStoreItem{FName{"ManaPotion"}, 30, false, FItemUIData{INVTEXT("Mana Potion")}});
		BackendStoreItems.Add(FStoreItem{FName{"Sword"}, 100, false, FItemUIData{INVTEXT("Sword")}});
		BackendStoreItems.Add(FStoreItem{FName{"Shield"}, 75, false, FItemUIData{INVTEXT("Shield")}});
		BackendStoreItems.Add(FStoreItem{FName{"Bow"}, 120, false, FItemUIData{INVTEXT("Bow")}});
		BackendStoreItems.Add(FStoreItem{FName{"ArrowPack"}, 20, false, FItemUIData{INVTEXT("Arrow Pack")}});
		BackendStoreItems.Add(FStoreItem{FName{"Helmet"}, 60, false, FItemUIData{INVTEXT("Helmet")}});
		BackendStoreItems.Add(FStoreItem{FName{"Armor"}, 150, false, FItemUIData{INVTEXT("Armor")}});
		BackendStoreItems.Add(FStoreItem{FName{"Boots"}, 40, false, FItemUIData{INVTEXT("Boots")}});
		BackendStoreItems.Add(FStoreItem{FName{"Ring"}, 80, false, FItemUIData{INVTEXT("Ring")}});
		BackendStoreItems.Add(FStoreItem{FName{"Amulet"}, 90, false, FItemUIData{INVTEXT("Amulet")}});
		BackendStoreItems.Add(FStoreItem{
			FName{"PotionOfStrength"}, 200, false, FItemUIData{INVTEXT("Potion of Strength")}
		});
		BackendStoreItems.Add(FStoreItem{
			FName{"PotionOfAgility"}, 150, false, FItemUIData{INVTEXT("Potion of Agility")}
		});
		BackendStoreItems.Add(FStoreItem{
			FName{"PotionOfIntelligence"}, 180, false, FItemUIData{INVTEXT("Potion of Intelligence")}
		});
		BackendStoreItems.Add(FStoreItem{
			FName{"ScrollOfFireball"}, 250, false, FItemUIData{INVTEXT("Scroll of Fireball")}
		});
		BackendStoreItems.Add(FStoreItem{
			FName{"ScrollOfTeleportation"}, 300, false, FItemUIData{INVTEXT("Scroll of Teleportation")}
		});
		BackendStoreItems.Add(FStoreItem{FName{"ElixirOfLife"}, 500, false, FItemUIData{INVTEXT("Elixir of Life")}});
		BackendStoreItems.Add(FStoreItem{FName{"MagicWand"}, 400, false, FItemUIData{INVTEXT("Magic Wand")}});
		BackendStoreItems.Add(FStoreItem{FName{"DragonScale"}, 600, false, FItemUIData{INVTEXT("Dragon Scale")}});
		BackendStoreItems.Add(FStoreItem{FName{"PhoenixFeather"}, 700, false, FItemUIData{INVTEXT("Phoenix Feather")}});
		BackendStoreItems.Add(FStoreItem{
			FName{"CrystalOfWisdom"}, 800, false, FItemUIData{INVTEXT("Crystal of Wisdom")}
		});
		BackendStoreItems.Add(FStoreItem{FName{"GoldenApple"}, 900, false, FItemUIData{INVTEXT("Golden Apple")}});
		BackendStoreItems.Add(FStoreItem{FName{"SilverCoin"}, 10, false, FItemUIData{INVTEXT("Silver Coin")}});
		BackendStoreItems.Add(FStoreItem{FName{"BronzeCoin"}, 5, false, FItemUIData{INVTEXT("Bronze Coin")}});
		BackendStoreItems.Add(FStoreItem{FName{"MysticGem"}, 250, false, FItemUIData{INVTEXT("Mystic Gem")}});
		BackendStoreItems.Add(FStoreItem{FName{"EnchantedBook"}, 300, false, FItemUIData{INVTEXT("Enchanted Book")}});
		BackendStoreItems.Add(FStoreItem{
			FName{"AncientArtifact"}, 1000, false, FItemUIData{INVTEXT("Ancient Artifact")}
		});
		BackendStoreItems.Add(FStoreItem{FName{"HealingHerb"}, 20, false, FItemUIData{INVTEXT("Healing Herb")}});
		BackendStoreItems.Add(FStoreItem{FName{"ManaHerb"}, 15, false, FItemUIData{INVTEXT("Mana Herb")}});
		BackendStoreItems.Add(FStoreItem{FName{"StaminaPotion"}, 40, false, FItemUIData{INVTEXT("Stamina Potion")}});
		BackendStoreItems.Add(FStoreItem{FName{"SpeedBoots"}, 70, false, FItemUIData{INVTEXT("Speed Boots")}});
		BackendStoreItems.Add(FStoreItem{FName{"StealthCloak"}, 120, false, FItemUIData{INVTEXT("Stealth Cloak")}});
		BackendStoreItems.Add(FStoreItem{FName{"BattleAxe"}, 200, false, FItemUIData{INVTEXT("Battle Axe")}});
		BackendStoreItems.Add(FStoreItem{FName{"Crossbow"}, 180, false, FItemUIData{INVTEXT("Crossbow")}});
		BackendStoreItems.Add(FStoreItem{FName{"MagicStaff"}, 350, false, FItemUIData{INVTEXT("Magic Staff")}});
		BackendStoreItems.Add(FStoreItem{FName{"HealingScroll"}, 220, false, FItemUIData{INVTEXT("Healing Scroll")}});
		BackendStoreItems.Add(FStoreItem{FName{"FireBomb"}, 150, false, FItemUIData{INVTEXT("Fire Bomb")}});
		BackendStoreItems.Add(FStoreItem{FName{"IceBomb"}, 160, false, FItemUIData{INVTEXT("Ice Bomb")}});
		BackendStoreItems.Add(FStoreItem{FName{"LightningBomb"}, 170, false, FItemUIData{INVTEXT("Lightning Bomb")}});
		BackendStoreItems.Add(FStoreItem{FName{"PoisonBomb"}, 140, false, FItemUIData{INVTEXT("Poison Bomb")}});
		BackendStoreItems.Add(FStoreItem{
			FName{"TeleportationStone"}, 300, false, FItemUIData{INVTEXT("Teleportation Stone")}
		});
		BackendStoreItems.Add(FStoreItem{FName{"MysticOrb"}, 400, false, FItemUIData{INVTEXT("Mystic Orb")}});
		BackendStoreItems.Add(FStoreItem{FName{"AncientScroll"}, 500, false, FItemUIData{INVTEXT("Ancient Scroll")}});
		BackendStoreItems.Add(FStoreItem{FName{"DragonEgg"}, 1000, false, FItemUIData{INVTEXT("Dragon Egg")}});
		BackendStoreItems.Add(FStoreItem{FName{"PhoenixEgg"}, 1200, false, FItemUIData{INVTEXT("Phoenix Egg")}});
		BackendStoreItems.Add(FStoreItem{FName{"CrystalBall"}, 600, false, FItemUIData{INVTEXT("Crystal Ball")}});
		BackendStoreItems.Add(FStoreItem{FName{"ElvenBow"}, 250, false, FItemUIData{INVTEXT("Elven Bow")}});
		BackendStoreItems.Add(FStoreItem{FName{"DwarvenHammer"}, 300, false, FItemUIData{INVTEXT("Dwarven Hammer")}});
		BackendStoreItems.Add(FStoreItem{FName{"HolyGrail"}, 800, false, FItemUIData{INVTEXT("Holy Grail")}});
		BackendStoreItems.Add(FStoreItem{FName{"CursedDagger"}, 400, false, FItemUIData{INVTEXT("Cursed Dagger")}});
		BackendStoreItems.Add(FStoreItem{FName{"VampireFang"}, 450, false, FItemUIData{INVTEXT("Vampire Fang")}});
		BackendStoreItems.Add(FStoreItem{FName{"WerewolfClaw"}, 500, false, FItemUIData{INVTEXT("Werewolf Claw")}});
	}
}

void UStoreSubsystem::CreateDummyOwnedStoreData()
{
	// Default owned items for the demo.
	BackendOwnedStoreItems.Empty(32);
	BackendOwnedStoreItems.Add(FStoreItem{FName{"LameSword"}, 5, true, FItemUIData{INVTEXT("Lame Sword")}});
	BackendOwnedStoreItems.Add(FStoreItem{FName{"BasicShield"}, 10, true, FItemUIData{INVTEXT("Basic Shield")}});
	BackendOwnedStoreItems.Add(FStoreItem{FName{"OldHelmet"}, 15, true, FItemUIData{INVTEXT("Old Helmet")}});
	BackendOwnedStoreItems.Add(FStoreItem{FName{"RustyBoots"}, 8, true, FItemUIData{INVTEXT("Rusty Boots")}});
	BackendOwnedStoreItems.Add(FStoreItem{FName{"WoodenBow"}, 12, true, FItemUIData{INVTEXT("Wooden Bow")}});
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

void UStoreSubsystem::BreakErrorState()
{
	if (IsValid(StoreViewModel) && StoreViewModel->HasStoreState(MolecularUITags::Store::State_Error))
	{
		StoreViewModel->SetErrorMessage(FText::GetEmpty());
		StoreViewModel->RemoveStoreState(MolecularUITags::Store::State_Error);
	}
}
