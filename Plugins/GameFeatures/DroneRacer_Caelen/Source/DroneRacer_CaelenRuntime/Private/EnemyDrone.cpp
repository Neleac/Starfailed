#include "EnemyDrone.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

#include "Asteroid.h"
#include "DroneCamera.h"
#include "DroneRacer_CaelenRuntime/GameModes/DroneGameMode.h"
#include "PlayerDrone.h"
#include "Projectile.h"

AEnemyDrone::AEnemyDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UStaticMeshComponent"));
	Mesh->SetupAttachment(RootComponent);

	Explosion = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("UParticleSystemComponent"));
	Explosion->SetupAttachment(RootComponent);

	PlayerDrone = nullptr;

	KillScore = 10.f;
	Speed = 10000.f;
	FireRate = 1.f;
	FireInterval = 0.f;
	CollisionDetectionDistance = 10000.f;
	bDebug = false;
	GoalDistanceThreshold = 1000.f;
}

void AEnemyDrone::BeginPlay()
{
	Super::BeginPlay();

	Box->OnComponentBeginOverlap.AddDynamic(this, &AEnemyDrone::BeginOverlap);
	Box->OnComponentHit.AddDynamic(this, &AEnemyDrone::OnHit);

	PlayerDrone = GameMode->GetPlayerDrone();

	// initial goal
	TTuple<float, float> EnemyPlaneRange = GameMode->GetEnemyPlaneRange();
	float GoalRelativeX = FMath::RandRange(EnemyPlaneRange.Get<0>(), EnemyPlaneRange.Get<1>());
	float GoalRelativeY = FMath::RandRange(-1.f, 1.f) * HorizontalLocationBound;
	float GoalRelativeZ = FMath::RandRange(-1.f, 1.f) * VerticalLocationBound;
	GoalRelativeLocation = FVector(GoalRelativeX, GoalRelativeY, GoalRelativeZ);
}

void AEnemyDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move(DeltaTime);
	Look();

	if (!bDebug)
	{
		Attack(DeltaTime);
	}
}

void AEnemyDrone::BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag(FName("PlayerBullet")))
	{
		Super::BeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

		if (Health <= 0.f)
		{
			PlayerDrone->AddScore(KillScore);

			// for multiplayer
			//Cast<AProjectile>(OtherActor)->GetProjector()->AddScore(KillScore);

			Death();
		}

		Cast<AProjectile>(OtherActor)->HitTarget();
	}
}

void AEnemyDrone::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

	if (Health <= 0.f)
	{
		PlayerDrone->AddScore(KillScore);

		// TODO: for multiplayer, only add score to player that hit asteroid into enemy

		Death();
	}
}

void AEnemyDrone::Death()
{
	Super::Death();

	Explosion->ActivateSystem();
	Mesh->SetVisibility(false);
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(false);

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemyDrone::DestroySelf, 1.5f, false);
}

void AEnemyDrone::DestroySelf()
{
	GameMode->DestroyEnemy();

	Destroy();
}

void AEnemyDrone::SetLocationBounds(float HorizontalBound, float VerticalBound)
{
	HorizontalLocationBound = HorizontalBound;
	VerticalLocationBound = VerticalBound;
}

void AEnemyDrone::Move(float DeltaTime)
{
	float MoveSpeed = Speed;
	
	FVector WorldLocation = GetActorLocation();
	FVector RelativeLocation = GameMode->WorldToCameraLocation(WorldLocation);

	FVector GoalWorldLocation = GameMode->CameraToWorldLocation(GoalRelativeLocation);
	FVector MoveDirection = (GoalWorldLocation - WorldLocation).GetSafeNormal();

	if (FVector::Dist(RelativeLocation, GoalRelativeLocation) < GoalDistanceThreshold)
	{
		if (bDebug)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Green, TEXT("GOAL REACHED"));
		}

		TTuple<float, float> EnemyPlaneRange = GameMode->GetEnemyPlaneRange();
		GoalRelativeLocation.X = FMath::RandRange(EnemyPlaneRange.Get<0>(), EnemyPlaneRange.Get<1>());
		GoalRelativeLocation.Y = FMath::RandRange(-1.f, 1.f) * HorizontalLocationBound;
		GoalRelativeLocation.Z = FMath::RandRange(-1.f, 1.f) * VerticalLocationBound;

		GoalWorldLocation = GameMode->CameraToWorldLocation(GoalRelativeLocation);
		MoveDirection = (GoalWorldLocation - WorldLocation).GetSafeNormal();
	}
	else
	{
		// avoid collisions
		FCollisionQueryParams TraceParams;
		TraceParams.bTraceComplex = false;
		TraceParams.AddIgnoredActor(this);

		FHitResult MoveHitResult;
		bool bMoveHit = GetWorld()->SweepSingleByChannel(
			MoveHitResult,
			WorldLocation,												// start
			WorldLocation + CollisionDetectionDistance * MoveDirection,	// end
			FQuat::Identity,											// rotation
			ECC_GameTraceChannel6,										// channel
			FCollisionShape::MakeSphere(8000.f),						// shape
			TraceParams
		);

		FHitResult BackHitResult;
		bool bBackHit = GetWorld()->SweepSingleByChannel(
			BackHitResult,
			WorldLocation,																	// start
			WorldLocation + CollisionDetectionDistance * Camera->GetActorForwardVector(),	// end
			FQuat::Identity,																// rotation
			ECC_GameTraceChannel6,															// channel
			FCollisionShape::MakeSphere(8000.f),											// shape
			TraceParams
		);

		if (bBackHit)
		{
			if (bDebug)
			{
				DrawDebugSphere(
					GetWorld(),
					BackHitResult.ImpactPoint,	// origin
					8000.f,						// radius
					16,							// segments
					FColor::Cyan,				// color
					false,						// persistent
					1.0f						// duration
				);

				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Cyan, TEXT("BACK ASTEROID"));
			}

			MoveSpeed *= 10.f;
		}

		//AAsteroid* Asteroid = Cast<AAsteroid>(HitResult.GetActor());
		//if (Asteroid)
		if (bMoveHit)
		{
			if (bDebug)
			{
				DrawDebugSphere(
					GetWorld(),
					MoveHitResult.ImpactPoint,	// origin
					8000.f,						// radius
					16,							// segments
					FColor::Magenta,			// color
					false,						// persistent
					1.0f						// duration
				);

				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Magenta, TEXT("MOVE ASTEROID"));
			}

			FVector RelativeImpactLocation = GameMode->WorldToCameraLocation(MoveHitResult.ImpactPoint);

			TTuple<float, float> EnemyPlaneRange = GameMode->GetEnemyPlaneRange();
			if (RelativeLocation.X < RelativeImpactLocation.X) {
				GoalRelativeLocation.X = FMath::RandRange(EnemyPlaneRange.Get<0>(), float(RelativeImpactLocation.X));
			}
			else
			{
				GoalRelativeLocation.X = FMath::RandRange(float(RelativeImpactLocation.X), EnemyPlaneRange.Get<1>());
			}

			if (RelativeLocation.Y < RelativeImpactLocation.Y) {
				GoalRelativeLocation.Y = FMath::RandRange(-HorizontalLocationBound, float(RelativeImpactLocation.Y));
			}
			else
			{
				GoalRelativeLocation.Y = FMath::RandRange(float(RelativeImpactLocation.Y), HorizontalLocationBound);
			}

			if (RelativeLocation.Z < RelativeImpactLocation.Z) {
				GoalRelativeLocation.Z = FMath::RandRange(-VerticalLocationBound, float(RelativeImpactLocation.Z));
			}
			else
			{
				GoalRelativeLocation.Z = FMath::RandRange(float(RelativeImpactLocation.Z), VerticalLocationBound);
			}
			
			GoalWorldLocation = GameMode->CameraToWorldLocation(GoalRelativeLocation);
			MoveDirection = (GoalWorldLocation - WorldLocation).GetSafeNormal();
		}
	}

	AddMovementInput(MoveDirection, MoveSpeed * DeltaTime);
}

void AEnemyDrone::Look()
{
	FVector* PastPlayerLocation = GameMode->GetPastPlayerLocation();

	FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), GameMode->CameraToWorldLocation(*PastPlayerLocation));
	SetActorRotation(Rotation);
}

void AEnemyDrone::Attack(float DeltaTime)
{
	if (FireInterval >= FireRate) {
		UDroneCamera* CameraComponent = Cast<UDroneCamera>(Camera->GetComponentByClass(UDroneCamera::StaticClass()));
		
		if (FMath::RandRange(0.f, 1.f) < 0.95f)
		{
			WeaponFire(CameraComponent->GetVelocity());
		}
		
		FireInterval -= FireRate;
	}
	FireInterval += DeltaTime;
}
