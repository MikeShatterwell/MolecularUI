// Copyright Mike Desrosiers, All Rights Reserved.

#include "MolecularUI.h"

#include <GameplayTagsManager.h>

#define LOCTEXT_NAMESPACE "FMolecularUIModule"

void FMolecularUIModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::ProjectPluginsDir() / TEXT("MolecularUI/Config/Tags"));
}

void FMolecularUIModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMolecularUIModule, MolecularUI)