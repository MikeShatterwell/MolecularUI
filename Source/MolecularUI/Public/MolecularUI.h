// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FMolecularUIModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
