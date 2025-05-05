// Fill out your copyright notice in the Description page of Project Settings.


#include "MViewModelBase.h"

void UMViewModelBase::HandleButtonPress()
{
	// Handle button press logic here
	UE_LOG(LogTemp, Warning, TEXT("%s Button Pressed!"), *GetName());
}

void UMViewModelBase::HandleButtonHover()
{
	// Handle button hovered logic here
	UE_LOG(LogTemp, Warning, TEXT("%s Button Hovered!"), *GetName());
}

void UMViewModelBase::HandleButtonUnhover()
{
	// Handle button unhovered logic here
	UE_LOG(LogTemp, Warning, TEXT("%s Button Unhovered!"), *GetName());
}
