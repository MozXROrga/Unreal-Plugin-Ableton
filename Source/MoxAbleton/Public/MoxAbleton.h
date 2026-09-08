// Copyright Ars Electronica Futurelab. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FMoxAbletonModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
