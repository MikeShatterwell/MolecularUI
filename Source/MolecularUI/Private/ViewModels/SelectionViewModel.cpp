// Copyright Mike Desrosiers, All Rights Reserved.

#include "ViewModels/SelectionViewModel.h"

#include "ViewModels/InteractiveViewModelBase.h"

// Setters that trigger field notifications
void USelectionViewModel::SetSelectionMode(const EMolecularSelectionMode InMode)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectionMode, InMode);
}

void USelectionViewModel::SetMaxSelectionCount(int32 InMaxCount)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxSelectionCount, FMath::Max(InMaxCount, 1));
}

void USelectionViewModel::SetSelectedViewModels(const TArray<UInteractiveViewModelBase*>& InSelected)
{
	UE_MVVM_SET_PROPERTY_VALUE(LastSelectedViewModel, InSelected.IsEmpty() ? nullptr : InSelected.Last());
	UE_MVVM_SET_PROPERTY_VALUE(SelectedViewModels, InSelected);
}

void USelectionViewModel::SetPreviewedViewModel(UInteractiveViewModelBase* InPreviewed)
{
	UE_MVVM_SET_PROPERTY_VALUE(PreviewedViewModel, InPreviewed);
}

// Main selection handlers
void USelectionViewModel::ToggleSelectViewModel(UInteractiveViewModelBase* ClickedViewModel)
{
	if (!IsValid(ClickedViewModel))
	{
		return;
	}

	TArray<UInteractiveViewModelBase*> NewSelection = SelectedViewModels;
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

void USelectionViewModel::PreviewViewModel(UInteractiveViewModelBase* HoveredViewModel)
{
	SetPreviewedViewModel(HoveredViewModel);
}

void USelectionViewModel::ClearPreview()
{
	SetPreviewedViewModel(nullptr);
}

void USelectionViewModel::ClearSelection()
{
	SetSelectedViewModels({});
}

// Helpers
bool USelectionViewModel::IsViewModelSelected(const UInteractiveViewModelBase* ViewModel) const
{
	return SelectedViewModels.Contains(ViewModel);
}