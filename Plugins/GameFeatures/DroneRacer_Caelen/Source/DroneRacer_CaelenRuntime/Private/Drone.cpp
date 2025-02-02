#include "Drone.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"

#include "Asteroid.h"
#include "DroneRacer_CaelenRuntime/GameModes/DroneGameMode.h"
#include "PlayerDrone.h"
#include "Projectile.h"

ADrone::ADrone()
{
	PrimaryActorTick.bCanEverTick = false;

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("UArrowComponent"));
	RootComponent = Arrow;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("UBoxComponent"));
	Box->SetupAttachment(RootComponent);

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("USceneComponent"));
	Muzzle->SetupAttachment(RootComponent);

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("UFloatingPawnMovement"));
	Movement->MaxSpeed = 100000.0f;
	Movement->Acceleration = 80000.0f;
	Movement->Deceleration = 80000.0f;

	ProjectileClass = nullptr;
	WeaponFireSound = nullptr;
	GameMode = nullptr;
	Camera = nullptr;
	MaxHealth = 100.f;
	
	SetLocationBounds();
}

float ADrone::GetCamDistToPlane()
{
	return CamDistToPlane;
}

void ADrone::SetCamDistToPlane(float Distance)
{
	CamDistToPlane = Distance;
}

void ADrone::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	GameMode = Cast<ADroneGameMode>(GetWorld()->GetAuthGameMode());
	check(GameMode);

	// attach to camera
	Camera = GameMode->GetCamera().Get();
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
	AttachToActor(Camera, AttachmentRules);
}

TTuple<float, float> ADrone::GetLocationBounds()
{
	return MakeTuple(HorizontalLocationBound, VerticalLocationBound);
}

void ADrone::SetLocationBounds()
{
	CamDistToPlane = 30000.f;
	HorizontalLocationBound = 30000.f;
	VerticalLocationBound = 15000.f;
	HorizontalBoundMargin = 5000.f;
	VerticalBoundMargin = 2500.f;
}

void ADrone::WeaponFire(const FVector& ProjectorVelocity, APlayerDrone* Projector)
{
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, Muzzle->GetComponentLocation(), Muzzle->GetComponentRotation(), ActorSpawnParams);
	Projectile->Fire(ProjectorVelocity);
	Projectile->SetProjector(Projector);

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, GetActorLocation(), GetActorRotation());
}

void ADrone::Death()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation(), GetActorRotation());
}

void ADrone::BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(AProjectile::StaticClass()))
	{
		Health -= Cast<AProjectile>(OtherActor)->GetDamage();
	}
}

void ADrone::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor->IsA(AAsteroid::StaticClass()))
	{
		//Health -= NormalImpulse.Size() / 10000000000;
		Health -= 10.f;
	}
}
