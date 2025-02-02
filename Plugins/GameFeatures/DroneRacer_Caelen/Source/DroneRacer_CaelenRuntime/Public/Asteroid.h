#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Asteroid.generated.h"

UCLASS()
class DRONERACER_CAELENRUNTIME_API AAsteroid : public AActor
{
	GENERATED_BODY()

	AActor* Camera;

	FVector InitialLocation;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;
	
public:	
	AAsteroid();
	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;
};
