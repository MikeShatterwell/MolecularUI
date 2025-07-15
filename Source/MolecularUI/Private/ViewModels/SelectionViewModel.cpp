// Copyright Mike Desrosiers, All Rights Reserved.

#include "ViewModels/SelectionViewModel.h"

// Setters that trigger field notifications
void USelectionViewModel::SetSelectionMode(const EMolecularSelectionMode InMode)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectionMode, InMode);
}

void USelectionViewModel::SetMaxSelectionCount(int32 InMaxCount)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxSelectionCount, InMaxCount);
}

void USelectionViewModel::SetSelectedViewModels(const TArray<UMVVMViewModelBase*>& InSelected)
{
	UE_MVVM_SET_PROPERTY_VALUE(LastSelectedViewModel, InSelected.IsEmpty() ? nullptr : InSelected.Last());
	UE_MVVM_SET_PROPERTY_VALUE(SelectedViewModels, InSelected);
}

void USelectionViewModel::SetPreviewedViewModel(UMVVMViewModelBase* InPreviewed)
{
	UE_MVVM_SET_PROPERTY_VALUE(PreviewedViewModel, InPreviewed);
}

// Main selection handlers
void USelectionViewModel::ToggleSelectViewModel(UMVVMViewModelBase* ClickedViewModel)
{
	if (!ClickedViewModel) return;

	TArray<UMVVMViewModelBase*> NewSelection = SelectedViewModels;
	const bool bIsAlreadySelected = NewSelection.Contains(ClickedViewModel);

	switch (SelectionMode)
	{
		case EMolecularSelectionMode::None:
			// Do nothing for selection
			break;

		case EMolecularSelectionMode::Single:
			NewSelection.Empty();
			NewSelection.Add(ClickedViewModel);
			break;

		case EMolecularSelectionMode::SingleToggle:
			if (bIsAlreadySelected)
			{
				NewSelection.Remove(ClickedViewModel);
			}
			else
			{
				NewSelection.Empty();
				NewSelection.Add(ClickedViewModel);
			}
			break;

		case EMolecularSelectionMode::Multi:
			if (bIsAlreadySelected)
			{
				NewSelection.Remove(ClickedViewModel);
			}
			else
			{
				NewSelection.Add(ClickedViewModel);
			}
			break;

		case EMolecularSelectionMode::MultiLimited:
			if (bIsAlreadySelected)
			{
				NewSelection.Remove(ClickedViewModel);
			}
			else if (NewSelection.Num() < MaxSelectionCount)
			{
				NewSelection.Add(ClickedViewModel);
			}
			break;
	}
	SetSelectedViewModels(NewSelection);
}

void USelectionViewModel::PreviewViewModel(UMVVMViewModelBase* HoveredViewModel)
{
	SetPreviewedViewModel(HoveredViewModel);
}

void USelectionViewModel::ClearPreview(UMVVMViewModelBase* UnhoveredViewModel)
{
	// Only clear the preview if the unhovered item is the currently previewed one
	if (PreviewedViewModel == UnhoveredViewModel)
	{
		SetPreviewedViewModel(nullptr);
	}
}

void USelectionViewModel::ClearSelection()
{
	SetSelectedViewModels({});
}

// Helpers
bool USelectionViewModel::IsViewModelSelected(const UMVVMViewModelBase* ViewModel) const
{
	return SelectedViewModels.Contains(ViewModel);
}