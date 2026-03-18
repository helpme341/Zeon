
#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "InputCoreTypes.h"
#include "Interact.generated.h"

UINTERFACE(BlueprintType)
class ZEON_API UInteract : public UInterface
{
	GENERATED_BODY()
};

class ZEON_API IInteract
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void Interact(const FGameplayTag& InteractType, const ETriggerEvent TriggerEvent, APawn* Player);
};
