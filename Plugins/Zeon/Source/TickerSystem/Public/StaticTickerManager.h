
#pragma once

#include "CoreMinimal.h"
#include "TickerModule.h"
#include "Engine/World.h"
#include "Containers/Ticker.h"
#include "Templates/SubclassOf.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StaticTickerManager.generated.h"

/** Enum для выбора ивента для активации или де активации тикера */
UENUM(BlueprintType)
enum class ETickerStateType : uint8
{
	Init,
	BeginPlay,
	GamePaused,

	EndPlay,
	GameUnPaused,
};

DECLARE_LOG_CATEGORY_EXTERN(LogStaticTicker, Log, All);

/** Класс, обеспечивающая централизованное управление логикой, работающей во времени через систему модулей. */
UCLASS(BlueprintType)
class TICKERSYSTEM_API UStaticTickerManager : public UObject
{
	GENERATED_BODY()

	friend UTickerModule;
	
	bool Tick(float DeltaTime);
	virtual void BeginDestroy() override;
	bool CleanupManager(float DeltaTime);
	
	bool DoesRequireTicker(const UTickerModule* IgnoreModule) const;
	
	void TryStartTicker();
	void TryEndTicker(const UTickerModule* Module) const;
	bool EndTicker() const;
	
	void OnGameStarted(EWorldType::Type WorldType);
	void OnGameEnded(UWorld* World);
	void OnGamePaused(bool bPaused);

	// ---------------- Vars ----------------

	bool bLastPauseState = false;
	float CurrentCleanupTime = 0.f;
	float CurrentPauseUpdateTime = 0.f;
	FTSTicker::FDelegateHandle TickHandle;
	FDelegateHandle GameEndedDelegateHandle;
	FDelegateHandle GameStartedDelegateHandle;
	FDelegateHandle GamePauseDelegateHandle;
	TMap<TSubclassOf<UTickerModule>, TStrongObjectPtr<UTickerModule>> TickerModules;
public:
	UStaticTickerManager();
	
	// ---------------- Settings ----------------

	/** Использовать ли систему проверки занятости тикера */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bUseCleanupSystem = true;

	/** Частота проверки занятость тикера */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (EditCondition = "bUseCleanupSystem"))
	float CleanupRate = 30.f;

	/** Частота обновления состояния паузы */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float PauseUpdateRate = 0.1f;

	/** Частота обновления главного тикера */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float GlobalTickerUpdateRate = 0.001;

	/** Список триггеров, при активации одного из них, система попытается активировать тикер */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSet<ETickerStateType> AutoActivateTickerType;

	/** Список триггеров, при активации одного из них, система попытается отключить тикер */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSet<ETickerStateType> AutoDisableTickerType;

	// ---------------- Fun ----------------

	/** Функция для добавления модуля в систему */
	UTickerModule* AddModule(const TSubclassOf<UTickerModule>& ModuleClass);

	/** Функция для добавления модулей в систему */
	FORCEINLINE void AddModules(const TSet<TSubclassOf<UTickerModule>>& ModuleClass)
	{
		for (auto Module : ModuleClass) AddModule(Module);
	}
		
	/** Функция для получения зарегистрированного в системе модуля */
	UTickerModule* GetModule(const TSubclassOf<UTickerModule>& ModuleClass);

	/** Функция для создания и регистрации модуля в системе через шаблон, важно что добавление должно происходить в конструкторе */
	template<typename T>
	T* AddModule();

	/** Функция для получения зарегистрированного в системе модуля через шаблон  */
	template<typename T>
	T* GetModule();

	/** Вызывает состояние активации или де активации тикера */
	FORCEINLINE void TryAutoModifyTickerState(const ETickerStateType& TickerState)
	{
		if (AutoActivateTickerType.Contains(TickerState)) TryStartTicker();
		if (AutoDisableTickerType.Contains(TickerState)) EndTicker();
	}

	/** Создает объект, важно отметить что функцию запрещённое юзать в конструкторе owner */
	static UStaticTickerManager* New(UObject* Owner) { return NewObject<UStaticTickerManager>(Owner); }
	
private:

	/** Обёртка для логов LogStaticTicker из шаблонов для избежания LNK ошибок. */
	FORCEINLINE static void LogTickerWarning(const FString& Msg)
	{
		UE_LOG(LogStaticTicker, Warning, TEXT("%s"), *Msg);
	}

	/** Обёртка для логов LogStaticTicker из шаблонов для избежания LNK ошибок. */
	FORCEINLINE static void LogTickerError(const FString& Msg)
	{
		UE_LOG(LogStaticTicker, Error, TEXT("%s"), *Msg);
	}
};

template <typename T>
T* UStaticTickerManager::AddModule()
{
	const auto& ModuleClass = T::StaticClass();
	if (TickerModules.Contains(ModuleClass))
	{
		LogTickerWarning(FString::Printf(TEXT("Cannot add module '%s' because it is already added"), *ModuleClass->GetName()));
		return nullptr;	
	}
	if (T* NewModule = NewObject<T>(this))
	{
		// Set up Module Settings...
		NewModule->OwnerManager = this;
		TickerModules.Add(ModuleClass, TStrongObjectPtr(MoveTemp(NewModule)));
		return NewModule;
	}
	LogTickerError(FString::Printf(TEXT("Cannot create module: %s"), *ModuleClass->GetName()));
	return nullptr;
}

template <typename T>
T* UStaticTickerManager::GetModule()
{
	const auto& ModuleClass = T::StaticClass();
	if (!TickerModules.Contains(ModuleClass))
	{
		LogTickerWarning(FString::Printf(TEXT("Cannot find module: %s"), *ModuleClass->GetName()));
		return nullptr;
	}
	TStrongObjectPtr<UTickerModule>* FoundPtr = TickerModules.Find(ModuleClass);
	return   FoundPtr ? Cast<T>(FoundPtr->Get()) : nullptr;
}