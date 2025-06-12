// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCommonUserWidget.h"

#include <Components/ProgressBar.h>
#include <Components/TextBlock.h>

void UMyCommonUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AMyCharacter* OwningCharacter = Cast<AMyCharacter>(GetOwningPlayerPawn()))
	{
		OwningCharacter->OnUpdateHealth.AddUObject(this, &UMyCommonUserWidget::OnUpdateHealth);
	}
}

void UMyCommonUserWidget::NativeDestruct()
{
	if (AMyCharacter* OwningCharacter = Cast<AMyCharacter>(GetOwningPlayerPawn()))
	{
		OwningCharacter->OnUpdateHealth.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UMyCommonUserWidget::OnUpdateHealth(const float NewHealth)
{
	if (NewHealth <= 0.0f)
	{
		// Handle death logic
		LabelText->SetText(FText::FromString("You are dead!"));
		LabelText->SetColorAndOpacity(FLinearColor::Red);
		ProgressBar->SetPercent(0.0f);
	}
	else
	{
		// Update health UI here
		const FText NewText = FText::Format(FText::FromString("Health: {0}"), FText::AsNumber(NewHealth));
		LabelText->SetText(NewText);
		LabelText->SetColorAndOpacity(FLinearColor::Green);
		ProgressBar->SetPercent(NewHealth / 100.0f);
	}
}
