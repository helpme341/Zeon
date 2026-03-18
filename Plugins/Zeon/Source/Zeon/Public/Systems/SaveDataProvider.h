
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "SaveDataProvider.generated.h"

UINTERFACE(BlueprintType)
class ZEON_API USaveDataProvider : public UInterface
{
	GENERATED_BODY()
};

class ZEON_API ISaveDataProvider
{
	GENERATED_BODY()

public:
	virtual FName GetSaveDomain() const = 0;
	virtual FGuid GetPersistentID() const = 0;

	virtual bool CaptureSaveData(FInstancedStruct& OutData) const = 0;
	virtual bool RestoreSaveData(const FInstancedStruct& InData) = 0;
};