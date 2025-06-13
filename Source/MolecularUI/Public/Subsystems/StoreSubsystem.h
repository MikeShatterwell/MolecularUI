#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "IStoreViewModelProvider.h"
#include "StoreTypes.h"
#include "StoreSubsystem.generated.h"

class UStoreViewModel;

UCLASS()
class UStoreSubsystem : public UGameInstanceSubsystem, public IStoreViewModelProvider
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// IStoreViewModelProvider implementation
	virtual UStoreViewModel* GetStoreViewModel_Implementation() override;

private:
	// Reacts to changes in the ViewModel's FilterText property.
	void OnFilterChanged();
    
	// Simulates fetching data asynchronously, demonstrating lazy loading.
	void LazyLoadStoreItems();

	// The single, authoritative instance of the Store ViewModel.
	UPROPERTY(Transient)
	TObjectPtr<UStoreViewModel> StoreViewModel;

	// The raw "model" data. In a real application, this would come from a database, file, or web API.
	UPROPERTY(Transient)
	TArray<FStoreItem> AllStoreItemsMasterList;

	FTimerHandle LoadTimerHandle;
};