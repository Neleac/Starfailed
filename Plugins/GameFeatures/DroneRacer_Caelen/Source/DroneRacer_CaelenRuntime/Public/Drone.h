#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Drone.generated.h"

class UArrowComponent;
class UBoxComponent;
class UFloatingPawnMovement;
class USoundBase;
class ADroneGameMode;
class APlayerDrone;
class AProjectile;

UCLASS()
class DRONERACER_CAELENRUNTIME_API ADrone : public APawn
{
	GENERATED_BODY()

public:
	ADrone();

	float GetCamDistToPlane();
	void SetCamDistToPlane(float Distance);

	TTuple<float, float> GetLocationBounds();

protected:
	/*
		COMPONENTS
	*/
	UPROPERTY(VisibleAnywhere)
	UArrowComponent* Arrow;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Box;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Muzzle;

	UPROPERTY(VisibleAnywhere)
	UFloatingPawnMovement* Movement;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	USoundBase* WeaponFireSound;

	UPROPERTY(EditAnywhere)
	USoundBase* DeathSound;

	/*
		WORLD
	*/
	ADroneGameMode* GameMode;
	AActor* Camera;

	/*
		DRONE
	*/
	UPROPERTY(EditAnywhere)
	float MaxHealth;
	float Health;

	// planar movement
	float CamDistToPlane;
	float HorizontalLocationBound;
	float VerticalLocationBound;

	UPROPERTY(EditAnywhere)
	float HorizontalBoundMargin;

	UPROPERTY(EditAnywhere)
	float VerticalBoundMargin;

	/*
		METHODS
	*/
	virtual void BeginPlay() override;

	virtual void SetLocationBounds();

	void WeaponFire(const FVector& ProjectorVelocity = FVector::ZeroVector, APlayerDrone* Projector = nullptr);

	virtual void Death();
	
	virtual void BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
