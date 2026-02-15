
#pragma once

#include "CoreMinimal.h"
#include "Logic.generated.h"

UINTERFACE(BlueprintType)
class LOGIC_API ULogic : public UInterface
{
	GENERATED_BODY()
};

class LOGIC_API ILogic
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Logic")
    bool GetIsActive();
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Logic")
    void SetIsActive(const bool bActive);
};
