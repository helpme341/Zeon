
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Delegates/Delegate.h"

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


public:
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
};