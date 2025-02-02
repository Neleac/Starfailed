#include "DroneGameMode.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Asteroid.h"
#include "EnemyDrone.h"
#include "PlayerDrone.h"

ADroneGameMode::ADroneGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), PlayerLocations(TQueue<FVector>())
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = nullptr;
	SpaceAmbienceSound = nullptr;
	SpaceMusic = nullptr;
	bGameStarted = false;

	PlayerDrone = nullptr;
	NumPlayerLocations = 0;

	MaxNumEnemies = 1;
	NumEnemies = 0;
	EnemyPlaneDistMin = 70000.f;
	EnemyPlaneDistMax = 100000.f;

	BP_EnemyDrone = NULL;
	AimDelaySeconds = 2.f;
}

void ADroneGameMode::BeginPlay()
{
	Super::BeginPlay();

	SpaceAmbienceAudio = UGameplayStatics::SpawnSound2D(GetWorld(), SpaceAmbienceSound);
}

void ADroneGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// spawn enemies
	if (bGameStarted && NumEnemies < MaxNumEnemies)
	{
		SpawnEnemy();
		NumEnemies++;
	}

	// keep fixed-size list of past player locations
	if (PlayerDrone)
	{
		int32 AimDelayFrames = 1.f / DeltaTime * AimDelaySeconds;
		if (NumPlayerLocations >= AimDelayFrames)
		{
			FVector PreviousPlayerLocation;
			PlayerLocations.Dequeue(PreviousPlayerLocation);
			NumPlayerLocations--;
		}
		PlayerLocations.Enqueue(WorldToCameraLocation(PlayerDrone->GetActorLocation()));
		NumPlayerLocations++;
	}
}

void ADroneGameMode::StartGame()
{
	bGameStarted = true;

	SpaceAmbienceAudio->SetActive(false);
	SpaceAudio = UGameplayStatics::SpawnSound2D(GetWorld(), SpaceMusic);
}

APlayerDrone* ADroneGameMode::GetPlayerDrone()
{
	return PlayerDrone;
}

void ADroneGameMode::SetPlayerDrone(APlayerDrone* Drone)
{
	PlayerDrone = Drone;
}

TSoftObjectPtr<AActor> ADroneGameMode::GetCamera()
{
	return Camera;
}

FVector* ADroneGameMode::GetPastPlayerLocation()
{
	return PlayerLocations.Peek();
}

void ADroneGameMode::SpawnEnemy()
{
	FVector CameraLocation = Camera->GetActorLocation();

	float PlayerPlaneDistance = PlayerDrone->GetCamDistToPlane();
	TTuple<float, float> PlayerLocationBounds = PlayerDrone->GetLocationBounds();

	float EnemyPlaneDistance = FMath::RandRange(EnemyPlaneDistMin, EnemyPlaneDistMax);
	float EnemyHorizontalBound = PlayerLocationBounds.Get<0>() * EnemyPlaneDistance / PlayerPlaneDistance;
	float EnemyVerticalBound = PlayerLocationBounds.Get<1>() * EnemyPlaneDistance / PlayerPlaneDistance;

	FVector EnemyWorldLocation = CameraLocation + EnemyPlaneDistance * Camera->GetActorForwardVector();
	FVector EnemyRelativeLocation = WorldToCameraLocation(EnemyWorldLocation);
	EnemyRelativeLocation.Y += FMath::RandRange(-1.f, 1.f) * EnemyHorizontalBound;
	EnemyRelativeLocation.Z += FMath::RandRange(-1.f, 1.f) * EnemyVerticalBound;
	EnemyWorldLocation = CameraToWorldLocation(EnemyRelativeLocation);
	
	FRotator EnemyRotation = UKismetMathLibrary::FindLookAtRotation(EnemyWorldLocation, CameraLocation);
	FActorSpawnParameters SpawnInfo;
	AEnemyDrone* EnemyDrone = GetWorld()->SpawnActor<AEnemyDrone>(BP_EnemyDrone, EnemyWorldLocation, EnemyRotation, SpawnInfo);
	EnemyDrone->SetCamDistToPlane(EnemyPlaneDistance);
	EnemyDrone->SetLocationBounds(EnemyHorizontalBound, EnemyVerticalBound);
}

void ADroneGameMode::DestroyEnemy()
{
	NumEnemies--;
}

TTuple<float, float> ADroneGameMode::GetEnemyPlaneRange()
{
	return MakeTuple(EnemyPlaneDistMin, EnemyPlaneDistMax);
}

FVector ADroneGameMode::WorldToCameraLocation(FVector WorldLocation)
{
	FTransform WorldTransform = FTransform(WorldLocation);
	FTransform CameraTransform = Camera->GetActorTransform();
	FTransform RelativeTransform = WorldTransform * CameraTransform.Inverse();
	FVector RelativeLocation = RelativeTransform.GetLocation();
	return RelativeLocation;
}

FVector ADroneGameMode::CameraToWorldLocation(FVector RelativeLocation)
{
	FTransform RelativeTransform = FTransform(RelativeLocation);
	FTransform CameraToWorldTransform = Camera->ActorToWorld();
	FTransform WorldTransform = RelativeTransform * CameraToWorldTransform;
	FVector WorldLocation = WorldTransform.GetLocation();
	return WorldLocation;
}
