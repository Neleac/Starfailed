#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraGameMode.h"
#include "DroneGameMode.generated.h"

class UAudioComponent;
class USoundBase;
class APlayerDrone;

UCLASS()
class DRONERACER_CAELENRUNTIME_API ADroneGameMode : public ALyraGameMode
{
	GENERATED_BODY()

	/*
		WORLD
	*/
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<AActor> Camera;

	UPROPERTY(EditAnywhere)
	USoundBase* SpaceAmbienceSound;
	UAudioComponent* SpaceAmbienceAudio;

	UPROPERTY(EditAnywhere)
	USoundBase* SpaceMusic;
	UAudioComponent* SpaceAudio;

	/*
		GAME
	*/
	bool bGameStarted;

	/*
		PLAYER
	*/
	APlayerDrone* PlayerDrone;
	TQueue<FVector> PlayerLocations;
	int32 NumPlayerLocations;

	/*
		ENEMY
	*/
	UPROPERTY(EditAnywhere)
	int32 MaxNumEnemies;
	int32 NumEnemies;

	UPROPERTY(EditAnywhere)
	float EnemyPlaneDistMin;

	UPROPERTY(EditAnywhere)
	float EnemyPlaneDistMax;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BP_EnemyDrone;

	UPROPERTY(EditAnywhere)
	float AimDelaySeconds;

public:
	ADroneGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void StartGame();

	TSoftObjectPtr<AActor> GetCamera();

	APlayerDrone* GetPlayerDrone();
	void SetPlayerDrone(APlayerDrone* Drone);

	FVector* GetPastPlayerLocation();

	void SpawnEnemy();
	void DestroyEnemy();

	TTuple<float, float> GetEnemyPlaneRange();

	FVector WorldToCameraLocation(FVector WorldLocation);
	FVector CameraToWorldLocation(FVector RelativeLocation);

protected:
	virtual void BeginPlay() override;
};
