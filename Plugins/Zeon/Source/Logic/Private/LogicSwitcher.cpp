
#include "LogicSwitcher.h"
#include "Net/UnrealNetwork.h"

ALogicSwitcher::ALogicSwitcher()
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("SwitcherSkeletalMesh");
	RootComponent = SkeletalMeshComponent;
}

void ALogicSwitcher::Switch()
{
	if (HasAuthority())
	{
		//bIsActivated = !bIsActivated;
	}
}

void ALogicSwitcher::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME(ALogicSwitcher, bIsActivated);
}

void ALogicSwitcher::BeginPlay()
{
	Super::BeginPlay();
	OnSwitched(false);
}