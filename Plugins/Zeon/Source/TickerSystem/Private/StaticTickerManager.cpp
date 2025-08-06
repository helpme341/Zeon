
#include "StaticTickerManager.h"
#include "UObject/UObjectGlobals.h"
#include "Utility/PauseManager.h"
#include "Utility/ZeonUtilits.h"
#include "TickerModule.h"

DEFINE_LOG_CATEGORY(LogStaticTicker);

UStaticTickerManager::UStaticTickerManager()
{
	GameStartedDelegateHandle = FZeonUtil::OnWorldBeginPlay.AddUObject(this, &UStaticTickerManager::OnGameStarted);
	GameEndedDelegateHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UStaticTickerManager::OnGameEnded);
	GamePauseDelegateHandle = FPauseManager::OnGamePause.AddUObject(this, &UStaticTickerManager::OnGamePaused);
	TryAutoModifyTickerState(ETickerStateType::Init);
}

void UStaticTickerManager::BeginDestroy()
{
	UObject::BeginDestroy();
	
	TickerModules.Empty();
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
	FZeonUtil::OnWorldBeginPlay.Remove(GameStartedDelegateHandle);
	FWorldDelegates::OnWorldBeginTearDown.Remove(GameEndedDelegateHandle);
	FPauseManager::OnGamePause.Remove(GamePauseDelegateHandle);
}


bool UStaticTickerManager::Tick(float DeltaTime)
{
	for (auto [_, Module] : TickerModules)
	{
		if (Module->bTickInPauseDisabled && !bLastPauseState) continue;
		Module->Tick(DeltaTime);
	}
	return !CleanupManager(DeltaTime);
}

bool UStaticTickerManager::CleanupManager(float DeltaTime)
{
	if (!TickHandle.IsValid() || !bUseCleanupSystem) return false;
	
	CurrentCleanupTime += DeltaTime;
	if (CleanupRate <= CurrentCleanupTime)
	{
		CurrentCleanupTime = 0.f;
		if (!DoesRequireTicker(nullptr))
		{
			UE_LOG(LogStaticTicker, Error, TEXT("Ticker leak: running without active tasks, forcibly stopped."));
			return true;
		}
	}
	return false;
}

bool UStaticTickerManager::DoesRequireTicker(const UTickerModule* IgnoreModule) const
{
	if (TickerModules.IsEmpty()) return false;
	for (auto& ModuleData : TickerModules)
	{
		if (ModuleData.Value->NeedUpdate())
		{
			if (IgnoreModule && ModuleData.Key != IgnoreModule->GetClass())
			{
				return true;
			}
		}
	}
	return false;
}

void UStaticTickerManager::TryStartTicker()
{
	if (TickHandle.IsValid())
	{
		UE_LOG(LogStaticTicker, Warning, TEXT("Cannot start ticker because it is already active"));
		return;
	}
	CurrentCleanupTime = 0.f;
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UStaticTickerManager::Tick), GlobalTickerUpdateRate);
}

void UStaticTickerManager::TryEndTicker(const UTickerModule* Module) const
{
	check(Module)
	if (!DoesRequireTicker(Module))
	{
		UE_LOG(LogStaticTicker, Warning, TEXT("Cannot disable ticker because module '%s' is using it"), *Module->GetClass()->GetName());
		return;
	}
	EndTicker();
}

bool UStaticTickerManager::EndTicker() const
{
	if (!TickHandle.IsValid())
	{
		UE_LOG(LogStaticTicker, Warning, TEXT("Cannot disable ticker because it is already disabled"));
		return false;
	}
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
	return !TickHandle.IsValid();
}

void UStaticTickerManager::OnGameStarted(EWorldType::Type /*WorldType*/)
{
	TryAutoModifyTickerState(ETickerStateType::BeginPlay);
	for (const auto& ModuleData : TickerModules) ModuleData.Value->OnGameStarted();
}

void UStaticTickerManager::OnGameEnded(UWorld* /*World*/)
{
	TryAutoModifyTickerState(ETickerStateType::EndPlay);
	for (const auto& ModuleData : TickerModules) ModuleData.Value->OnGameEnded();
}

void UStaticTickerManager::OnGamePaused(bool bPaused)
{
	TryAutoModifyTickerState(bPaused ? ETickerStateType::GamePaused : ETickerStateType::GameUnPaused);
	for (const auto& ModuleData : TickerModules)
	{
		ModuleData.Value->bIsGamePaused = bPaused;
		bPaused ? ModuleData.Value->OnGamePaused() : ModuleData.Value->OnGameUnPaused();
	}
}


UTickerModule* UStaticTickerManager::AddModule(const TSubclassOf<UTickerModule>& ModuleClass)
{
	if (TickerModules.Contains(ModuleClass))
	{
		UE_LOG(LogStaticTicker, Warning, TEXT("Cannot add module '%s' because it is already added"), *ModuleClass->GetName());
		return nullptr;	
	}
	if (UTickerModule* NewModule = NewObject<UTickerModule>(this, ModuleClass.Get()))
	{
		// Setting Module Settings...
		NewModule->OwnerManager = this;
		TickerModules.Add(ModuleClass, TStrongObjectPtr(MoveTemp(NewModule)));
		return NewModule;
	}
	UE_LOG(LogStaticTicker, Error, TEXT("Cannot create module: %s"), *ModuleClass->GetName());
	return nullptr;
}

UTickerModule* UStaticTickerManager::GetModule(const TSubclassOf<UTickerModule>& ModuleClass)
{
	if (!TickerModules.Contains(ModuleClass))
	{
		UE_LOG(LogStaticTicker, Warning, TEXT("Cannot find module: %s"), *ModuleClass->GetName());
		return nullptr;
	}
	return TickerModules.Find(ModuleClass)->Get();
}