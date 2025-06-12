// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <GameFramework/Character.h>

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MyCommonUserWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UButton;

DECLARE_MULTICAST_DELEGATE_OneParam(FUpdateHealth, float /*Health*/);

/**
 * 
 */
UCLASS()
class MOLECULARUI_API UMyCommonUserWidget : public UCommonUserWidget
{
	GENERATED_BODY()
public:

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Bound to the character health update event
	void OnUpdateHealth(float NewHealth);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar = nullptr;
};

UCLASS()
class AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	FUpdateHealth OnUpdateHealth;
};
