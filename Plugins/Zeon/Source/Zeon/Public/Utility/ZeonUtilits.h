
#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Delegates/Delegate.h"
#include "StructUtils/StructView.h"


namespace FZeonMath
{
		/** 
	 * Структура описывает уклон поверхности вдоль направления скорости.
	 * Используется для определения, поднимается ли персонаж (или транспорт) по склону, 
	 * спускается, и насколько круто этот уклон.
	 */
	struct FSignedSlope
	{
		/** 
		 * Угол наклона поверхности вдоль направления скорости, в градусах, со знаком.
		 * > 0 — движение вверх по склону.
		 * < 0 — движение вниз по склону.
		 * ≈ 0 — горизонтальная поверхность.
		 */
		float SignedAngleDeg = 0.f;

		/** 
		 * Тангенс угла уклона (отношение подъёма к горизонтальному расстоянию) со знаком.
		 * Это фактически «градиент» поверхности: 
		 *   + → движение вверх по склону,
		 *   -- → движение вниз.
		 *
		 * Пример: значение 0.5 означает подъём на 0.5 метра на каждый метр вперёд (~26.5°).
		 */
		float SignedGrade = 0.f;

		/** 
		 * Флаг, показывающий направление относительно склона:
		 * true  — движение вверх по склону;
		 * false — движение вниз или по горизонтали.
		 * 
		 * Это упрощённая форма проверки (SignedGrade > 0).
		 */
		bool bUphill = false;
	};

	static FSignedSlope GetSignedSlopeAlongVelocity(const FVector& N, const FVector& V, const FVector& Up = FVector::UpVector)
	{
		FSignedSlope Out;

		if (!N.IsNormalized()) return Out;
		if (V.IsNearlyZero())  return Out;

		const float cosTheta = FMath::Clamp(FVector::DotProduct(N, Up), -1.f, 1.f);
		const float slopeAngleRad = FMath::Acos(cosTheta);
		const float slopeTan      = FMath::Tan(slopeAngleRad);

		if (slopeTan <= KINDA_SMALL_NUMBER) return Out;
		
		const FVector Descent = FVector::VectorPlaneProject(-Up, N).GetSafeNormal();
		if (Descent.IsNearlyZero()) return Out;

		const FVector Ascent = -Descent;
		
		const FVector VOnPlane = FVector::VectorPlaneProject(V, N);
		const FVector DirOnPlane = VOnPlane.GetSafeNormal();
		if (DirOnPlane.IsNearlyZero()) return Out;
		
		const float uphillDot = FVector::DotProduct(DirOnPlane, Ascent);

		const float signedGrade    = slopeTan * uphillDot;
		const float signedAngleRad = FMath::Atan(signedGrade);

		Out.SignedGrade    = signedGrade;
		Out.SignedAngleDeg = FMath::RadiansToDegrees(signedAngleRad);
		Out.bUphill        = (signedGrade > 0.01f);
		return Out;
	}

	static FVector GetDirectionAlongImpactNormal(const FVector& N, const FVector& V)
	{
		if (!N.IsNormalized() || V.IsNearlyZero()) return FVector::ZeroVector;
		const FVector VOnPlane = FVector::VectorPlaneProject(V, N);
		return VOnPlane.IsNearlyZero() ? FVector::ZeroVector : VOnPlane.GetSafeNormal();
	}
	
	

}


class ZEON_API FZeonUtil
{
	static TUniquePtr<FZeonUtil> Instance;
	static FDelegateHandle PostWorldInitDelegateHandle;
	
	static void OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues /*IVS*/)
	{
		World->OnWorldBeginPlay.AddLambda([WorldType = World->WorldType]{ OnWorldBeginPlay.Broadcast(WorldType); });
	}

public:

	static void Initialize()
	{
		if (!Instance) Instance = MakeUnique<FZeonUtil>();
		PostWorldInitDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddStatic(&OnPostWorldInitialization);
	}
	
	static void Shutdown()
	{
		Instance.Reset();
		OnWorldBeginPlay.Clear();
		FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitDelegateHandle);
	}

	static FZeonUtil& Get()
	{
		check(Instance)
		return *Instance;
	}

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnWorldBeginPlay, EWorldType::Type /*WorldType*/);
	static FOnWorldBeginPlay OnWorldBeginPlay;
	

	FORCEINLINE static const TSet<EWorldType::Type>& GetDefaultWorldTypes()
	{
		static const TSet Types = { EWorldType::PIE, EWorldType::Game };
		return Types;
	}
	static UWorld* FindWorld(const TSet<EWorldType::Type>& WorldTypes = GetDefaultWorldTypes())
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (WorldTypes.Contains(Context.WorldType)) return Context.World();
		}
		return nullptr;
	}

	static bool AreHitsEqual(const FHitResult& A, const FHitResult& B, float Tolerance = KINDA_SMALL_NUMBER)
	{
		return A.ImpactPoint.Equals(B.ImpactPoint, Tolerance)
			&& A.ImpactNormal.Equals(B.ImpactNormal, Tolerance);
	}

	

	static bool CapsuleTraceAtLocation(
		const UCapsuleComponent* Capsule, const float RadiusBonus, const float HalfBonus,
		const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
	    FHitResult& OutHit, const bool bTraceComplex = false, const bool bShowDebag = false)
	{
	    if (!Capsule) return false;

	    const UWorld* World = Capsule->GetWorld();
	    if (!World) return false;

	    const FVector Center = Capsule->GetComponentLocation();
	    const FQuat   Rot    = Capsule->GetComponentQuat();

	    const float Radius = Capsule->GetScaledCapsuleRadius() + RadiusBonus;
	    const float Half   = Capsule->GetScaledCapsuleHalfHeight() + HalfBonus;
	    const FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius, Half);

	    FCollisionQueryParams Params(SCENE_QUERY_STAT(CapsuleTraceAtLocation), bTraceComplex);
	    Params.AddIgnoredActor(Capsule->GetOwner());

	    FCollisionObjectQueryParams ObjParams;
	    for (auto OT : ObjectTypes)
	        ObjParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(OT.GetValue()));

	    TArray<FHitResult> Hits;
	    const bool bAny = World->SweepMultiByObjectType(Hits, Center, Center, Rot, ObjParams, Shape, Params);
	    if (!bAny) return false;

	    Hits.Sort([](const FHitResult& A, const FHitResult& B)
	    {
	        const bool ABlock = !!A.bBlockingHit;
	        const bool BBlock = !!B.bBlockingHit;
	        if (ABlock != BBlock) return ABlock;

	        const bool APen = !!A.bStartPenetrating;
	        const bool BPen = !!B.bStartPenetrating;
	        if (APen != BPen) return APen;

	        if (APen && BPen)
	        {
	            if (A.PenetrationDepth != B.PenetrationDepth) return A.PenetrationDepth > B.PenetrationDepth; 
	        }
	        return A.Time < B.Time;
	    });

	    OutHit = Hits[0];
		if (bShowDebag)
		{
			const FColor Color = OutHit.bBlockingHit ? FColor::Red : FColor::Yellow;

			DrawDebugCapsule(World, Center, Half, Radius, Rot, Color, false, 1.f, 0, 1.f);
			DrawDebugPoint(World, OutHit.ImpactPoint, 12.f, Color, false, 1.f);
			DrawDebugLine(World, OutHit.ImpactPoint,
						  OutHit.ImpactPoint + OutHit.ImpactNormal * 30.f,
						  Color, false, 1.f, 0, 1.f);
		}
	    return true;
	}

	static bool CapsuleTraceAtLocation(
		const UCapsuleComponent* Capsule, const FVector& Center, const float RadiusBonus, const float HalfBonus,
		const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
		FHitResult& OutHit, const bool bTraceComplex = false, const bool bShowDebag = false)
	{
		if (!Capsule) return false;

		const UWorld* World = Capsule->GetWorld();
		if (!World) return false;

		const FQuat   Rot    = Capsule->GetComponentQuat();

		const float Radius = Capsule->GetScaledCapsuleRadius() + RadiusBonus;
		const float Half   = Capsule->GetScaledCapsuleHalfHeight() + HalfBonus;
		const FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius, Half);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(CapsuleTraceAtLocation), bTraceComplex);
		Params.AddIgnoredActor(Capsule->GetOwner());

		FCollisionObjectQueryParams ObjParams;
		for (auto OT : ObjectTypes)
			ObjParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(OT.GetValue()));

		TArray<FHitResult> Hits;
		const bool bAny = World->SweepMultiByObjectType(Hits, Center, Center, Rot, ObjParams, Shape, Params);
		if (!bAny) return false;

		Hits.Sort([](const FHitResult& A, const FHitResult& B)
		{
			const bool ABlock = !!A.bBlockingHit;
			const bool BBlock = !!B.bBlockingHit;
			if (ABlock != BBlock) return ABlock;

			const bool APen = !!A.bStartPenetrating;
			const bool BPen = !!B.bStartPenetrating;
			if (APen != BPen) return APen;

			if (APen && BPen)
			{
				if (A.PenetrationDepth != B.PenetrationDepth) return A.PenetrationDepth > B.PenetrationDepth; 
			}
			return A.Time < B.Time;
		});

		OutHit = Hits[0];
		if (bShowDebag)
		{
			const FColor Color = OutHit.bBlockingHit ? FColor::Red : FColor::Yellow;

			DrawDebugCapsule(World, Center, Half, Radius, Rot, Color, false, 1.f, 0, 1.f);
			DrawDebugPoint(World, OutHit.ImpactPoint, 12.f, Color, false, 1.f);
			DrawDebugLine(World, OutHit.ImpactPoint,
						  OutHit.ImpactPoint + OutHit.ImpactNormal * 30.f,
						  Color, false, 1.f, 0, 1.f);
		}
		return true;
	}
	
	
	static bool AreTransformsNearlyEqual(
		const FTransform& A, const FTransform& B,
		const float LocationTolerance = 1.0f,        // сантиметры
		const float RotationToleranceDeg = 1.0f,     // градусы
		const float ScaleTolerance = 0.01f)
	{
		const bool bLocation = A.GetLocation().Equals(B.GetLocation(), LocationTolerance);
		const bool bRotation = A.GetRotation().AngularDistance(B.GetRotation()) <= FMath::DegreesToRadians(RotationToleranceDeg);
		const bool bScale = A.GetScale3D().Equals(B.GetScale3D(), ScaleTolerance);

		return bLocation && bRotation && bScale;
	}

	static FTransform InterpTransformTo(
		const FTransform& Current,
		const FTransform& Target,
		const float DeltaTime,
		const float Speed,
		const bool bConstant = false
	)
	{
		FTransform Out = Current;

		const FVector NewLoc = bConstant
			? FMath::VInterpConstantTo(Current.GetLocation(), Target.GetLocation(), DeltaTime, Speed)
			: FMath::VInterpTo(Current.GetLocation(), Target.GetLocation(), DeltaTime, Speed);

		const FQuat NewRot = bConstant
			? FMath::QInterpConstantTo(Current.GetRotation(), Target.GetRotation(), DeltaTime, Speed)
			: FMath::QInterpTo(Current.GetRotation(), Target.GetRotation(), DeltaTime, Speed);

		const FVector NewScale = bConstant
			? FMath::VInterpConstantTo(Current.GetScale3D(), Target.GetScale3D(), DeltaTime, Speed)
			: FMath::VInterpTo(Current.GetScale3D(), Target.GetScale3D(), DeltaTime, Speed);

		Out.SetLocation(NewLoc);
		Out.SetRotation(NewRot);
		Out.SetScale3D(NewScale);

		return Out;
	}

};