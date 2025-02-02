#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UParticleSystemComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class APlayerDrone;

UCLASS()
class DRONERACER_CAELENRUNTIME_API AProjectile : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionVolume;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere)
	float Damage;

	APlayerDrone* Projector;

	void Death();

public:	
	AProjectile();

	virtual void Tick(float DeltaTime) override;

	void Fire(const FVector& ProjectorVelocity = FVector::ZeroVector);

	void HitTarget();

	float GetDamage();

	void SetProjector(APlayerDrone* Drone);

	APlayerDrone* GetProjector();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* HitParticle;
};
