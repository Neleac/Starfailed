#include "Asteroid.h"

#include "Kismet/GameplayStatics.h"

#include "DroneRacer_CaelenRuntime/GameModes/DroneGameMode.h"
#include "PlayerDrone.h"
#include "Projectile.h"

AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UStaticMeshComponent"));
	RootComponent = Mesh;
}

void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	ADroneGameMode* GameMode = Cast<ADroneGameMode>(GetWorld()->GetAuthGameMode());
	check(GameMode);
	Camera = GameMode->GetCamera().Get();

	InitialLocation = GetActorLocation();

	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AAsteroid::BeginOverlap);
}

void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Mesh->IsSimulatingPhysics())
	{
		FVector CameraToAsteroid = GetActorLocation() - Camera->GetActorLocation();
		float AsteroidVisibility = FVector::DotProduct(CameraToAsteroid, Camera->GetActorForwardVector());

		FVector CameraToReset = InitialLocation - Camera->GetActorLocation();
		float ResetVisibility = FVector::DotProduct(CameraToReset, Camera->GetActorForwardVector());
		
		// reset asteroid if too far or out of view
		if (AsteroidVisibility < 0.707f && ResetVisibility < 0.707f)
		{
			Mesh->SetSimulatePhysics(false);
			Mesh->SetCollisionProfileName(FName("OverlapAllDynamic"));
			SetActorLocation(InitialLocation);
		}
	}
}

void AAsteroid::BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Mesh->IsSimulatingPhysics())
	{
		Mesh->SetSimulatePhysics(true);
		Mesh->SetCollisionProfileName(FName("PhysicsActor"));
	}

	if (OtherActor->IsA(AProjectile::StaticClass()))
	{
		Cast<AProjectile>(OtherActor)->HitTarget();
		Mesh->AddImpulseAtLocation(OtherActor->GetVelocity() * 1000000.f, OtherActor->GetActorLocation());
	}
}
