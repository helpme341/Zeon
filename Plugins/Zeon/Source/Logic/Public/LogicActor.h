 #pragma once

#include "CoreMinimal.h"
#include "LogicActor.generated.h"

class ALogicActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogicFinished, const ALogicActor*, LogicActor);

UCLASS(ClassGroup="Logic")
class LOGIC_API ALogicActor : public AActor
{
    GENERATED_BODY()

public:
    ALogicActor();

    /** Dependency list that must finish before this actor is considered finished. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Logic")
    TArray<TObjectPtr<ALogicActor>> Dependencies;

    /** Returns true when this LogicActor is finished (replicated). */
   // UFUNCTION(BlueprintCallable, Category="Logic")
   ///  bool IsFinished() const { return bFinished; }

    /**
     * Server-only: attempts to finish this actor if all dependencies are finished.
     * Safe to call multiple times; it will finish only once.
     */
    UFUNCTION(BlueprintCallable, Category="Logic")
    void TryFinish();

    /** Fired when this logic actor becomes finished (server immediately; clients via OnRep). */
    UPROPERTY(BlueprintAssignable, Category="Logic")
    FOnLogicFinished OnLogicFinished;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    /** Replicated finished state. */
    UPROPERTY(ReplicatedUsing=OnRep_IsActivated)
    bool bIsActivated = false;

    /** Client-side notification when bFinished becomes true. */
    UFUNCTION()
    void OnRep_IsActivated();
 
    /** Called on server when a dependency finishes. */
    UFUNCTION()
    void HandleDependencyFinished(const ALogicActor* FinishedDependency);

    /** Server helper: checks whether all (valid) dependencies have finished. */
    bool AreAllDependenciesFinished() const;

    /** Server-only set of finished dependencies (not replicated). */
    TSet<TWeakObjectPtr<const ALogicActor>> FinishedDependencies;
};
