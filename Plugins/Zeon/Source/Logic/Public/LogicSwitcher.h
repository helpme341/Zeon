 #pragma once

#include "CoreMinimal.h"
#include "LogicActor.h"
#include "LogicSwitcher.generated.h"

UCLASS(ClassGroup="Logic")
class LOGIC_API ALogicSwitcher : public ALogicActor
{
	GENERATED_BODY()
	 
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void OnSwitched(bool bActivated);
	virtual void OnSwitched_Implementation(bool bActivated) {}

	//UPROPERTY(ReplicatedUsing=OnRep_IsActivated)
	//bool bIsActivated = false;
	
	//UFUNCTION()
	//void OnRep_IsActivated() { OnSwitched(bIsActivated); }
public:
	ALogicSwitcher();
	
	UFUNCTION(BlueprintCallable)
	void Switch();
};
