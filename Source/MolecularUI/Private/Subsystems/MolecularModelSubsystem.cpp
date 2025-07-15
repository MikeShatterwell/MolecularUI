// Copyright Mike Desrosiers, All Rights Reserved.

#include "Subsystems/MolecularModelSubsystem.h"

#include "Models/MolecularModelBase.h"
#include "Utils/LogMolecularUI.h"

bool UMolecularModelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && 
			Outer
			&& Outer->GetWorld()
			&& Outer->GetWorld()->IsGameWorld();
}

void UMolecularModelSubsystem::Deinitialize()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	for (auto& [ModelClass, ModelObject] : ModelInstances)
	{
		if (IsValid(ModelObject))
		{
			ModelObject->DeinitializeModel();
		}
	}
	ModelInstances.Empty();
	Super::Deinitialize();
}

UMolecularModelBase* UMolecularModelSubsystem::GetModel(const TSubclassOf<UMolecularModelBase> ModelClass)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	if (ModelClass == nullptr)
	{
		return nullptr;
	}

	if (TObjectPtr<UMolecularModelBase>* FoundModel = ModelInstances.Find(ModelClass))
	{
		// Return already initialized model instance
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Found existing model instance of type %s"), 
			__FUNCTION__, *ModelClass->GetName());
		return *FoundModel;
	}

	if (UMolecularModelBase* NewModel = NewObject<UMolecularModelBase>(this, ModelClass))
	{
		UE_LOG(LogMolecularUI, Log, TEXT("[%hs] Created new model instance of type %s"), 
			__FUNCTION__, *ModelClass->GetName());
		NewModel->InitializeModel(GetWorld());
		ModelInstances.Add(ModelClass, NewModel);
		return NewModel;
	}

	return nullptr;
}
