#pragma once

#include "CoreMinimal.h"
#include "Drone.h"
#include "EnemyDrone.generated.h"

class UParticleSystemComponent;
class UStaticMeshComponent;
class APlayerDrone;

UCLASS()
class DRONERACER_CAELENRUNTIME_API AEnemyDrone : public ADrone
{
	GENERATED_BODY()

	APlayerDrone* PlayerDrone;

	UPROPERTY(EditAnywhere)
	int32 KillScore;

	UPROPERTY(EditAnywhere)
	float Speed;

	UPROPERTY(EditAnywhere)
	float FireRate;
	float FireInterval;

	UPROPERTY(EditAnywhere)
	float CollisionDetectionDistance;

	UPROPERTY(EditAnywhere)
	bool bDebug;

	FVector GoalRelativeLocation;
	float GoalDistanceThreshold;

	//virtual void SetLocationBounds() override;

	void Move(float DeltaTime);

	void Look();

	void Attack(float DeltaTime);

	UFUNCTION()
	virtual void BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void Death() override;

	void DestroySelf();

public:
	AEnemyDrone();

	virtual void Tick(float DeltaTime) override;

	void SetLocationBounds(float HorizontalBound, float VerticalBound);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* Explosion;
};
