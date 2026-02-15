
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/StructView.h"
#include "SettingsHelper.generated.h"

struct FConstStructView;

/**
 * Lightweight reference to item settings stored as:
 *  - CustomSettings: owned copy (safe for BP/serialization usage)
 *  - SettingsView: non-owning view to external struct memory (NO lifetime guarantees)
 *
 * If bCacheCopyForBlueprint is enabled, SetFromView() will also cache an owned copy
 * into CustomSettings so the data is safe to use from Blueprint and stable across lifetime.
 */
USTRUCT(BlueprintType)
struct ZEON_API FItemSettingsRef
{
	GENERATED_BODY()


	FORCEINLINE bool IsValid() const
	{
		return CustomSettings.IsValid() || SettingsView.IsValid();
	}

	/** Clears owned copy and invalidates the view. */
	FORCEINLINE void Reset()
	{
		CustomSettings.Reset();
		SettingsView = FConstStructView();
	}

	/**
	 * Binds to an external settings struct via view.
	 * IMPORTANT: SettingsView does NOT own memory. Caller must guarantee lifetime.
	 * Optionally caches an owned copy into CustomSettings.
	 */
	FORCEINLINE void SetFromView(const FConstStructView& InView)
	{
		SettingsView = InView;

		if (bCacheCopyForBlueprint && InView.IsValid())
		{
			const auto Struct = InView.GetScriptStruct();
			CustomSettings.InitializeAs(Struct);
			Struct->CopyScriptStruct(CustomSettings.GetMutableMemory(), InView.GetMemory());
		}
	}

	template<typename T>
	FORCEINLINE const T* Get() const
	{
		if (CustomSettings.IsValid()) return CustomSettings.GetPtr<T>();
		return SettingsView.GetPtr<const T>();
	}

private:
	/** Owned copy of settings (safe for BP and stable lifetime). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FInstancedStruct CustomSettings;

	/** If true, SetFromView() caches an owned copy into CustomSettings for BP usage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bCacheCopyForBlueprint = false;

	/** Non-owning view to external settings struct memory. Lifetime must be guaranteed by the source. */
	FConstStructView SettingsView;
};

template <typename TClass>
class TSettingsHelper
{
protected:
	/** Функция для проверки является ли объект CDO, обязана быть реализована в наследнике класса. */
	virtual bool IsThisCDO() const = 0;
	
	/** Функция нахождения Data Table с настройками, обязана быть реализована в наследнике класса. */
	virtual UDataTable* FindDataTable() const = 0;
	
 	/** Кешированный Data Table с настройками. */
	TWeakObjectPtr<UDataTable> DataTableSettings;
public:
	virtual ~TSettingsHelper() = default;
	
	/** Функия для нахождения настроек по имени. */
	FConstStructView FindSettingsByName(const FName& Name) const
	{
		auto DataTable = DataTableSettings.Get();
		if (IsThisCDO()) DataTable = FindDataTable();
		if (!DataTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("DataTable is NULL"));
			return FConstStructView();
		}

		if (const uint8* Src = DataTable->FindRowUnchecked(Name))
		{
			return FConstStructView(DataTable->GetRowStruct(), Src);
		}
		UE_LOG(LogTemp, Error, TEXT("Settings not found for name '%s' in DataTable '%s'."), *Name.ToString(), *DataTableSettings->GetName());
		return FConstStructView();
	}

	/** Функия для нахождения настроек по имени. */
	template<typename T>
	const T* FindSettingsByName(const FName& Name) const;
	
	/** Функия для нахождения настроек по классу. */
	FORCEINLINE FConstStructView FindSettingsByClass(const TSubclassOf<TClass>& Class) const
	{
		if (!Class) return FConstStructView();
		if (const auto* Object = Class->template GetDefaultObject<TClass>()) return FindSettingsByName(Object->GetSettingsName());
		return FConstStructView();
	}
	
	/** Функия для нахождения настроек по классу. */
	template<typename T>
	const T* FindSettingsByClass(const TSubclassOf<TClass>& Class) const;
};

template <typename TClass>
template <typename T>
const T* TSettingsHelper<TClass>::FindSettingsByName(const FName& Name) const
{
	auto DataTable = DataTableSettings.Get();
	if (IsThisCDO()) DataTable = FindDataTable();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("DataTable is NULL"));
		return nullptr;
	}

	return DataTable->FindRow<T>(Name, "Settings not found");
}

template <typename TClass>
template <typename T>
const T* TSettingsHelper<TClass>::FindSettingsByClass(const TSubclassOf<TClass>& Class) const
{
	if (!Class) return nullptr;
	if (const auto* Object = Class->template GetDefaultObject<TClass>()) return FindSettingsByName<T>(Object->GetSettingsName());
	return nullptr;
}