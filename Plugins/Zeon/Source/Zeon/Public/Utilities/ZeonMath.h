
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Delegates/Delegate.h"


namespace FTraceUtility
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
	
	static bool AreHitsEqual(const FHitResult& A, const FHitResult& B, float Tolerance = KINDA_SMALL_NUMBER)
	{
		return A.ImpactPoint.Equals(B.ImpactPoint, Tolerance)
			&& A.ImpactNormal.Equals(B.ImpactNormal, Tolerance);
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

}
