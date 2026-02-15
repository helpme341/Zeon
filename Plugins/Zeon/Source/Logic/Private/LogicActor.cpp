
#include "LogicActor.h"
#include "Net/UnrealNetwork.h"

ALogicActor::ALogicActor()
{
    bReplicates = true;
}

void ALogicActor::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;
    for (ALogicActor* Dep : Dependencies)
    {
        if (!IsValid(Dep)) continue;

        Dep->OnLogicFinished.RemoveDynamic(this, &ALogicActor::HandleDependencyFinished);
        Dep->OnLogicFinished.AddDynamic(this, &ALogicActor::HandleDependencyFinished);

       // if (Dep->IsFinished()) HandleDependencyFinished(Dep);
    }
}

void ALogicActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    //DOREPLIFETIME(ALogicActor, bFinished);
}

void ALogicActor::OnRep_IsActivated()
{
  //  if (bFinished) OnLogicFinished.Broadcast(this);
}

void ALogicActor::HandleDependencyFinished(const ALogicActor* FinishedDependency)
{
    if (!HasAuthority() || !IsValid(FinishedDependency)) return;

    const TWeakObjectPtr Key(FinishedDependency);
    if (FinishedDependencies.Contains(Key)) return;

    FinishedDependencies.Add(Key);
    TryFinish();
}

bool ALogicActor::AreAllDependenciesFinished() const
{
    for (const ALogicActor* Dep : Dependencies)
    {
        if (!IsValid(Dep)) continue;
        if (!FinishedDependencies.Contains(TWeakObjectPtr(Dep)))  return false;
    }
    return true;
}

void ALogicActor::TryFinish()
{
    if (!HasAuthority()) return;
   // if (bFinished) return;
    if (!AreAllDependenciesFinished()) return;

   // bFinished = true;
    OnLogicFinished.Broadcast(this);
}
