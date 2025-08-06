
#include "TickerModule.h"
#include "StaticTickerManager.h"

void UTickerModule::TryStartTicker() const
{
	OwnerManager->TryStartTicker();
}

void UTickerModule::TryEndTicker() const
{
	OwnerManager->TryEndTicker(this);
}

void UTickerModule::TryEndTickerSave() const
{
	if (!NeedUpdate()) OwnerManager->TryEndTicker(this);
}
