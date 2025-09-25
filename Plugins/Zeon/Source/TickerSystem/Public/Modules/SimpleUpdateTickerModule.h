
#pragma once

#include "CoreMinimal.h"
#include "TickerModule.h"
#include "Utility/Invoker.h"

struct FSimpleUpdateTask
{
	FSimpleUpdateTask(const float InUpdateRate)
		: UpdateRate(InUpdateRate) {}
	
	float UpdateRate;
	float RemainingTime = 0.0f;
};

class TICKERSYSTEM_API FSimpleUpdateTickerModule : public FTickerModule
{
	virtual void Tick(float DeltaTime) override
	{
		TArray<FName> CompletedTasks;
		for (auto& TaskData : UpdateTasks)
		{
			const auto& Key = TaskData.Key;
			auto& Settings = TaskData.Value;

			Settings.RemainingTime += DeltaTime;
			if (Settings.RemainingTime >= Settings.UpdateRate)
			{
				Settings.RemainingTime = 0.f;
				if (!ActivateTaskInvoker(Key, DeltaTime)) CompletedTasks.Add(Key);
			}
		}
		for (auto Key : CompletedTasks) UpdateTasks.Remove(Key);
	}
	virtual bool NeedUpdate() const override
	{
		return !UpdateTasks.IsEmpty();
	}
	
	TMap<FName, FSimpleUpdateTask> UpdateTasks;
public:
	virtual ~FSimpleUpdateTickerModule() override
	{
		UpdateTasks.Empty();
	}
	
	TInvoker<bool(const FName&, const float)> ActivateTaskInvoker;

	FORCEINLINE void AddTask(const FName& Key, const float UpdateRate)
	{
		if (UpdateTasks.Contains(Key)) return;
		TryStartTicker();
		UpdateTasks.Add(Key, FSimpleUpdateTask(UpdateRate));
	}
	FORCEINLINE void RemoveTask(const FName& Key)
	{
		if (!UpdateTasks.Contains(Key)) return;
		TryEndTicker();
		UpdateTasks.Remove(Key);
	}
};
