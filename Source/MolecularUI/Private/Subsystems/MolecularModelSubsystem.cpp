// Copyright Mike Desrosiers, All Rights Reserved.

#include "Subsystems/MolecularModelSubsystem.h"

#include "Models/MolecularModelBase.h"

bool UMolecularModelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && 
			Outer
			&& Outer->GetWorld()
			&& Outer->GetWorld()->IsGameWorld();
}

UMolecularModelBase* UMolecularModelSubsystem::GetModel(const TSubclassOf<UMolecularModelBase> ModelClass)
{
	if (ModelClass == nullptr)
	{
		return nullptr;
	}

	if (TObjectPtr<UMolecularModelBase>* FoundModel = ModelInstances.Find(ModelClass))
	{
		return *FoundModel;
	}

	if (UMolecularModelBase* NewModel = NewObject<UMolecularModelBase>(this, ModelClass))
	{
		NewModel->InitializeModel(GetWorld());
		ModelInstances.Add(ModelClass, NewModel);
		return NewModel;
	}

	return nullptr;
}
