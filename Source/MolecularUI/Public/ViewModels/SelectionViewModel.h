// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "MolecularTypes.h"
#include "MVVMViewModelBase.h"

#include "SelectionViewModel.generated.h"

class UInteractiveViewModelBase;
/*
 * A ViewModel for managing selections in a UI, supporting single, multi, and limited multi-selection modes.
 *
 * This was created to replace the default ListView/TileView selection handling (handled internally on the widget) with
 * a system that's more compatible with the MVVM plugin, allowing the ViewModel to drive selection state.
 * 
 */
UCLASS(Blueprintable)
class USelectionViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	// Getters and Setters for View properties
	UFUNCTION(BlueprintPure, FieldNotify, Category="Selection ViewModel")
	EMolecularSelectionMode GetSelectionMode() const { return SelectionMode; }

	UFUNCTION(BlueprintCallable, Category="Selection ViewModel")
	void SetSelectionMode(EMolecularSelectionMode InMode);

	UFUNCTION(BlueprintPure, FieldNotify, Category="Selection ViewModel")
	int32 GetMaxSelectionCount() const { return MaxSelectionCount; }
	
	UFUNCTION(BlueprintCallable, Category = "Selection ViewModel")
	void SetMaxSelectionCount(int32 InMaxCount);

	UFUNCTION(BlueprintPure, FieldNotify, Category="Selection ViewModel")
	const TArray<UInteractiveViewModelBase*>& GetSelectedViewModels() const { return SelectedViewModels; }
	
	UFUNCTION(BlueprintPure, FieldNotify, Category="Selection ViewModel")
	UInteractiveViewModelBase* GetPreviewedViewModel() const { return PreviewedViewModel; }

	UFUNCTION(BlueprintPure, FieldNotify, Category="Selection ViewModel")
	UInteractiveViewModelBase* GetLastSelectedViewModel() const { return LastSelectedViewModel; }

	// Main selection handlers
	UFUNCTION(BlueprintCallable, Category="Selection ViewModel")
	void ToggleSelectViewModel(UInteractiveViewModelBase* ClickedViewModel);

	UFUNCTION(BlueprintCallable, Category="Selection ViewModel")
	void PreviewViewModel(UInteractiveViewModelBase* HoveredViewModel);

	UFUNCTION(BlueprintCallable, Category = "Selection ViewModel")
	void ClearPreview();

	UFUNCTION(BlueprintCallable, Category = "Selection ViewModel")
	void ClearSelection();

	// Helpers
	UFUNCTION(BlueprintCallable, Category = "Selection ViewModel")
	bool IsViewModelSelected(const UInteractiveViewModelBase* ViewModel) const;

private:
	void SetSelectedViewModels(const TArray<UInteractiveViewModelBase*>& InSelected);
	void SetPreviewedViewModel(UInteractiveViewModelBase* InPreviewed);

	UPROPERTY(BlueprintReadWrite, FieldNotify, meta=(AllowPrivateAccess=true), Category="Selection ViewModel")
	TArray<TObjectPtr<UInteractiveViewModelBase>> SelectedViewModels;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, meta=(AllowPrivateAccess=true), Category="Selection ViewModel")
	TObjectPtr<UInteractiveViewModelBase> LastSelectedViewModel = nullptr;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, meta=(AllowPrivateAccess=true), Category="Selection ViewModel")
	TObjectPtr<UInteractiveViewModelBase> PreviewedViewModel = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = true), Category = "Selection ViewModel")
	EMolecularSelectionMode SelectionMode = EMolecularSelectionMode::Single;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = true, EditCondition = "SelectionMode == ESelectionMode::MultiLimited"), Category = "Selection ViewModel")
	int32 MaxSelectionCount = 1;
};