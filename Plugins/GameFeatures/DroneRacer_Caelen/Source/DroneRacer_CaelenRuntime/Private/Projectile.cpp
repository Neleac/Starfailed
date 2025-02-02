#include "Projectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"

#include "PlayerDrone.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	CollisionVolume->InitSphereRadius(5.0f);
	CollisionVolume->SetGenerateOverlapEvents(true);
	CollisionVolume->CanCharacterStepUpOn = ECB_No;	
	CollisionVolume->SetCollisionProfileName(FName("OverlapAllDynamic"));
	RootComponent = CollisionVolume;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UStaticMeshComponent"));
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->CanCharacterStepUpOn = ECB_No;
	Mesh->SetCollisionProfileName(FName("NoCollision"));
	Mesh->SetupAttachment(RootComponent);

	HitParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("UParticleSystemComponent"));
	HitParticle->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 10000.f;
	ProjectileMovement->MaxSpeed = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bAutoActivate = false;

	InitialLifeSpan = 3.0f;

	Damage = 11.f;

	Projector = nullptr;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::Fire(const FVector& ProjectorVelocity)
{
	ProjectileMovement->Velocity = ProjectileMovement->InitialSpeed * GetActorForwardVector() + ProjectorVelocity;
	ProjectileMovement->Activate();
}

void AProjectile::HitTarget()
{
	HitParticle->ActivateSystem();
	Mesh->SetVisibility(false);
	CollisionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(false);

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AProjectile::Death, 0.5f, false);
}

void AProjectile::Death()
{
	Destroy();
}

float AProjectile::GetDamage()
{
	return Damage;
}

void AProjectile::SetProjector(APlayerDrone* Drone)
{
	Projector = Drone;
}

APlayerDrone* AProjectile::GetProjector()
{
	return Projector;
}
