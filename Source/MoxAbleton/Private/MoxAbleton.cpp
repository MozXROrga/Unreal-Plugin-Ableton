// Copyright Ars Electronica Futurelab. All Rights Reserved.

#include "MoxAbleton.h"
#include "MoxAbletonTypes.h"

#define LOCTEXT_NAMESPACE "FMoxAbletonModule"

// Define log category
DEFINE_LOG_CATEGORY(LogMoxAbleton);

void FMoxAbletonModule::StartupModule()
{
	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton Module loaded"));
}

void FMoxAbletonModule::ShutdownModule()
{
	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton Module unloaded"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMoxAbletonModule, MoxAbleton)