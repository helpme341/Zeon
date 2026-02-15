
#pragma once

#include "CoreMinimal.h"
#include "Logic.h"
#include "Utility/ZeonUtilits.h"
#include "MovingActor.generated.h"

UCLASS(BlueprintType)
class LOGIC_API AMovingActor : public AActor, public ILogic
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings")
	bool bIsActive = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|InterpolationSettings")
	bool bUseConstantInterpolation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|InterpolationSettings")
	float InterpolationSpeed = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|InterpolationSettings")
	float LoopDelay = 1.0f;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|LocationSettings")
	bool bInterpolateLocation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|LocationSettings", meta = (EditCondition = "bInterpolateLocation", EditConditionHides))
	FVector TargetLocation = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|LocationSettings", meta = (EditCondition = "bInterpolateLocation", EditConditionHides))
	float LocationTolerance = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|LocationSettings", meta = (EditCondition = "bInterpolateLocation", EditConditionHides))
	bool bUseTargetAsOffset = true;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|RotationSettings")
	bool bInterpolateRotation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|RotationSettings", meta = (EditCondition = "bInterpolateRotation", EditConditionHides))
	FRotator TargetRotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|RotationSettings", meta = (EditCondition = "bInterpolateRotation", EditConditionHides))
	float RotationTolerance = 1.f;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|ScaleSettings")
	bool bInterpolateScale = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|ScaleSettings", meta = (EditCondition = "bInterpolateScale", EditConditionHides))
	FVector TargetScale = FVector::OneVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingSettings|ScaleSettings", meta = (EditCondition = "bInterpolateScale", EditConditionHides))
	float ScaleTolerance = 1.f;
	
	FORCEINLINE virtual void SetIsActive_Implementation(const bool bActive) override { bIsActive = bActive; }
	FORCEINLINE virtual bool GetIsActive_Implementation() override { return bIsActive; }
	virtual void BeginPlay() override
	{
		Super::BeginPlay();
		StarterTransform = GetActorTransform();
	}


	virtual void Tick(float DeltaTime) override
	{
		Super::Tick(DeltaTime);
		if (bIsActive)
		{
			if (LoopReloadTime != 0.f)
			{
				LoopReloadTime = FMath::Max(0.f, LoopReloadTime - DeltaTime);
				if (LoopReloadTime > 0.f) return;
			}
			
			FTransform TargetTransform;
			const auto CurrentTargetLocation = bReturning ? StarterTransform.GetLocation() : bUseTargetAsOffset ? StarterTransform.GetLocation() + TargetLocation : TargetLocation;
			const auto CurrentTargetRotation = bReturning ? StarterTransform.GetRotation() : TargetRotation.Quaternion();
			const auto CurrentTargetScale = bReturning ? StarterTransform.GetScale3D() : TargetScale;
			
			TargetTransform.SetLocation(bInterpolateLocation ? CurrentTargetLocation : GetActorLocation());
			TargetTransform.SetRotation(bInterpolateRotation ? CurrentTargetRotation : GetActorQuat());
			TargetTransform.SetScale3D(bInterpolateScale ? CurrentTargetScale : GetActorScale3D());
	
			if (FZeonUtil::AreTransformsNearlyEqual(TargetTransform, GetActorTransform(), LocationTolerance, RotationTolerance, ScaleTolerance))
			{
				bReturning = !bReturning;
				LoopReloadTime = LoopDelay;
			}
			else
			{
				TargetTransform = FZeonUtil::InterpTransformTo(GetActorTransform(), TargetTransform, DeltaTime, InterpolationSpeed, bUseConstantInterpolation);
				SetActorTransform(TargetTransform);
			}
		}
	}
		
	bool bReturning = false;
	float LoopReloadTime = 0.f;
	FTransform StarterTransform;
public:
	AMovingActor()
	{
		PrimaryActorTick.bCanEverTick = true;
		
		StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("StaticMeshComp"));
		StaticMeshComp->SetCollisionObjectType(ECC_WorldStatic);
		RootComponent = StaticMeshComp;
	}
};