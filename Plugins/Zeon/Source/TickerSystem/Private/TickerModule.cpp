
#include "TickerModule.h"
#include "StaticTickerManager.h"

void FTickerModule::TryStartTicker() const
{
	OwnerManager->TryStartTicker();
}

void FTickerModule::TryEndTicker() const
{
	OwnerManager->TryEndTicker(this);
}

void FTickerModule::TryEndTickerSave() const
{
	if (!NeedUpdate()) TryEndTicker();
}
