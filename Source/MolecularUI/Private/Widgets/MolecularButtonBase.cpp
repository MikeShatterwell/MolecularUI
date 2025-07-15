// Copyright Mike Desrosiers, All Rights Reserved.

#include "Widgets/MolecularButtonBase.h"

void UMolecularButtonBase::HandleButtonClicked()
{
	if (!bUseStatefulInteraction)
	{
		Super::HandleButtonClicked();
	}
	else
	{
		if (IsInteractionEnabled())
		{
			if (bRequiresHold && CurrentHoldProgress < 1.f)
			{
				return;
			}
			NativeOnClicked();
			ExecuteTriggeredInput();
			HoldReset();
		}
	}
}
