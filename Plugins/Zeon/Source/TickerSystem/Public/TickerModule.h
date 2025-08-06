
#pragma once

#include "CoreMinimal.h"
#include "TickerModule.generated.h"

/** Обработчик задачи, подключённый к FStaticTickerManager для выполнения во времени */
UCLASS()
class TICKERSYSTEM_API UTickerModule : public UObject
{
	GENERATED_BODY()
	
	friend class UStaticTickerManager;

	bool bIsGamePaused = false;
	
	/** Владелец - менеджер модуля */
	UPROPERTY()
	TObjectPtr<UStaticTickerManager> OwnerManager;
protected:
	FORCEINLINE bool GetIsGamePaused() const { return bIsGamePaused; }

	/** Функция, которая вызывается каждый тик (или настроенное во владельце время),
	 * также важно отметить что тик в менеджере может быть выключен и вызов этой функции подкрутится. */
	virtual void Tick(float DeltaTime) {}

	/** Вызывается в начале игры */
	virtual void OnGameStarted() {}
	/** Вызывается в конце игры */
	virtual void OnGameEnded() {}
	/** Вызывается при начале паузы во время игры */
	virtual void OnGamePaused() {}
	/** Вызывается при окончании паузы во время игры */
	virtual void OnGameUnPaused() {}
	
	/** Вызывается из менеджера для проверки нужен ли модулю tick, если нужен то возвращаем true, иначе false */
	virtual bool NeedUpdate() const PURE_VIRTUAL(UTickerModule::NeedUpdate, return false;)

	/** Функция для попытки начать работу tich в менеджера */	
	void TryStartTicker() const;
	/** Функция для попытки закончить работу tich в менеджере */	
	void TryEndTicker() const;

	/** Функция для попытки закончить работу tich в менеджере, с проверкой нужен ли тикер этому модулю */	
	void TryEndTickerSave() const;

	bool bTickInPauseDisabled = true;
};