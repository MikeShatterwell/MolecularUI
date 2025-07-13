// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <GameplayTagContainer.h>

#include <CoreMinimal.h>
#include "CommonButtonBase.h"
#include "MolecularButtonBase.generated.h"

/**
 * A base class for buttons that provide an interaction source for InteractiveViewModel stateful communication.
 */
UCLASS()
class MOLECULARUI_API UMolecularButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	/*
	 * The source of the interaction (e.g., "InteractionSource.TabList", "InteractionSource.DetailsPanel").
	 * This is a tool to solve a problem in MVVM where the same ViewModel is used in multiple contexts and the source of the interaction is needed to differentiate them.
	 * An empty tag means the interaction is not from a specific source, or the source is not relevant.
	 *
	 * When set, this should be bound to the ViewModel's Interaction.Source property to allow the ViewModel to differentiate between interactions from different sources.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Molecular UI", meta = (Categories = "InteractionSource"))
	FGameplayTag InteractionSource = FGameplayTag::EmptyTag;
};
