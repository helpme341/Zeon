
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "SaveProvider.generated.h"

UINTERFACE(BlueprintType)
class ZEON_API USaveProvider : public UInterface
{
	GENERATED_BODY()
};

class ZEON_API ISaveProvider
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SaveProvider")
	virtual FName GetSaveDomain() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SaveProvider")
	virtual FGuid GetPersistentID() const = 0;

	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SaveProvider")
	virtual bool CaptureSaveData(FInstancedStruct& OutData) const = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SaveProvider")
	virtual bool RestoreSaveData(const FInstancedStruct& InData) = 0;
};