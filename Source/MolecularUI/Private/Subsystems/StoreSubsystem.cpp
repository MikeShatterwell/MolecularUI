// Source/StoreSystem/Private/StoreSubsystem.cpp
#include "Subsystems/StoreSubsystem.h"
#include "ViewModels/StoreViewModel.h"
#include "ViewModels/ItemViewModel.h"
#include "TimerManager.h"

void UStoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Create the main ViewModel instance.
	StoreViewModel = NewObject<UStoreViewModel>(this);
	
	// DEMONSTRATES STATEFUL COMMUNICATION: Subsystem (Model) binds to ViewModel changes.
	// When the UI changes the FilterText in the ViewModel, the Subsystem will be notified and can react.
	if (StoreViewModel)
	{
		// Get the FFieldId for the "FilterText" property.
		 UE::FieldNotification::FFieldId FieldId = UItemViewModel::FFieldNotificationClassDescriptor::ItemData;
		if (FieldId.IsValid())
		{
			
			StoreViewModel->AddFieldValueChangedDelegate(FieldId, INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &UStoreSubsystem::OnFilterChanged));
		}
	}

	// Populate with some dummy data for the demo.
	AllStoreItemsMasterList.Add({FGuid(), LOCTEXT("HealthPotion", "Health Potion"), 50});
	AllStoreItemsMasterList.Add({FGuid(), LOCTEXT("ManaPotion", "Mana Potion"), 75});
	AllStoreItemsMasterList.Add({FGuid(), LOCTEXT("SwordOfPower", "Sword of Power"), 1200});
	AllStoreItemsMasterList.Add({FGuid(), LOCTEXT("ShieldOfValor", "Shield of Valor"), 950});
}

void UStoreSubsystem::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(LoadTimerHandle);
	}
	StoreViewModel = nullptr;
	Super::Deinitialize();
}

UStoreViewModel* UStoreSubsystem::GetStoreViewModel_Implementation()
{
	// On first request from the UI, start loading the data.
	// Subsequent calls will just return the existing instance.
	if (StoreViewModel && StoreViewModel->GetItems().Num() == 0 && !StoreViewModel->GetbIsLoading())
	{
		UE_MVVM_SET_PROPERTY_VALUE(StoreViewModel, bIsLoading, true);

		// LAZY LOADING PATTERN: Simulate a network/disk delay.
		GetWorld()->GetTimerManager().SetTimer(LoadTimerHandle, this, &UStoreSubsystem::LazyLoadStoreItems, 1.5f, false);
	}
	return StoreViewModel;
}

void UStoreSubsystem::OnFilterChanged()
{
	// The UI changed the filter text, so we re-apply our data logic.
	LazyLoadStoreItems();
}

void UStoreSubsystem::LazyLoadStoreItems()
{
	if (!StoreViewModel)
	{
		return;
	}

	TArray<TScriptInterface<UItemViewModel>> FilteredItems;
	const FString CurrentFilter = StoreViewModel->GetFilterText();

	for (const FStoreItem& ItemData : AllStoreItemsMasterList)
	{
		// If filter is empty or item name contains the filter, add it.
		if (CurrentFilter.IsEmpty() || ItemData.DisplayName.ToString().Contains(CurrentFilter))
		{
			UItemViewModel* ItemVM = NewObject<UItemViewModel>(StoreViewModel);
			ItemVM->SetData(ItemData.ItemId, ItemData.DisplayName, ItemData.Price, ItemData.Icon);
			FilteredItems.Add(ItemVM);
		}
	}

	// Update the ViewModel's properties. The UI will automatically react to these changes.
	UE_MVVM_SET_PROPERTY_VALUE(StoreViewModel, Items, FilteredItems);
	UE_MVVM_SET_PROPERTY_VALUE(StoreViewModel, bIsLoading, false);
}