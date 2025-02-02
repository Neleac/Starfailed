#include "PlayerDrone.h"

#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "DroneCamera.h"
#include "DroneRacer_CaelenRuntime/GameModes/DroneGameMode.h"
#include "Projectile.h"

APlayerDrone::APlayerDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	DevMode = false;

	ScreenX = 1920;
	ScreenY = 1080;
	OffsetX = 0.f;
	OffsetY = 0.f;

	// mouse controls drone orientation
	bool bUseControllerRot = true;
	bUseControllerRotationPitch = bUseControllerRot;
	bUseControllerRotationYaw = bUseControllerRot;
	bUseControllerRotationRoll = bUseControllerRot;

	Score = 0;

	MouseInactive = 0.f;

	AsteroidImpactSound = nullptr;
}

void APlayerDrone::BeginPlay()
{
	Super::BeginPlay();

	GameMode->SetPlayerDrone(this);

	// register collision
	Box->OnComponentBeginOverlap.AddDynamic(this, &APlayerDrone::BeginOverlap);
	Box->OnComponentHit.AddDynamic(this, &APlayerDrone::OnHit);

	// attach StarSparrow
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
	StarSparrow->AttachToActor(this, AttachmentRules);
}

void APlayerDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!DevMode)
	{
		ConstrainLocation();
		SetRotation();

		MouseInactive += DeltaTime;
	}
}

void APlayerDrone::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerController = GetController<APlayerController>();

	// setup player input
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	Subsystem->AddMappingContext(InputMappingContext, 0);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerDrone::Input_Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerDrone::Input_LookMouse);
	EnhancedInputComponent->BindAction(WeaponFireAction, ETriggerEvent::Triggered, this, &APlayerDrone::Input_WeaponFire);

	// constrain movement to viewport
	PlayerController->GetViewportSize(ScreenX, ScreenY);
	ConstrainViewportAspectRatio();
	SetLocationBounds();
}

void APlayerDrone::Input_Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (DevMode)
	{
		AddMovementInput(GetActorForwardVector(), 0.2f * MovementVector.Y);
		AddMovementInput(GetActorRightVector(), 0.2f * MovementVector.X);
	}
	else
	{
		AddMovementInput(Camera->GetActorUpVector(), MovementVector.Y);
		AddMovementInput(Camera->GetActorRightVector(), MovementVector.X);
		//AddControllerRollInput(-0.5 * MovementVector.X);
		if (MouseInactive > 1.f)
		{
			AddControllerPitchInput(MovementVector.Y);
			AddControllerYawInput(MovementVector.X);
		}
	}
}

void APlayerDrone::Input_LookMouse(const FInputActionValue& Value)
{
	if (DevMode)
	{
		FVector2D LookAxisVector = Value.Get<FVector2D>();
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
	else
	{
		MouseInactive = 0.f;
	}
}

void APlayerDrone::Input_WeaponFire(const FInputActionValue& Value)
{
	if (DevMode)
	{
		WeaponFire(GetVelocity(), this);
	}
	else
	{
		UDroneCamera* CameraComponent = Cast<UDroneCamera>(Camera->GetComponentByClass(UDroneCamera::StaticClass()));
		WeaponFire(CameraComponent->GetVelocity(), this);
	}
}

void APlayerDrone::BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag(FName("EnemyBullet")))
	{
		Super::BeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

		OnHealthChanged.Broadcast(Health / MaxHealth);

		Cast<AProjectile>(OtherActor)->HitTarget();

		if (Health <= 0.f)
		{
			Death();
		}
	}
}

void APlayerDrone::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), AsteroidImpactSound, OtherActor->GetActorLocation(), OtherActor->GetActorRotation());

	OnHealthChanged.Broadcast(Health / MaxHealth);

	if (Health <= 0.f)
	{
		Death();
	}
}

void APlayerDrone::SetLocationBounds()
{
	// screen space to world space projection lands on plane near to camera
	FVector WorldLocation;
	FVector WorldDirection;
	FVector2D TopLeftCorner(OffsetX, OffsetY);
	UGameplayStatics::DeprojectScreenToWorld(PlayerController, TopLeftCorner, WorldLocation, WorldDirection);

	// use desired drone plane distance to camera to calculate drone world location
	float CosTheta = FVector::DotProduct(WorldDirection, Camera->GetActorForwardVector());
	float CamDistToDroneOnNearPlane = FVector::Dist(WorldLocation, Camera->GetActorLocation());
	float CamDistToNearPlane = CamDistToDroneOnNearPlane * CosTheta;
	FVector CamToDrone = GetActorLocation() - Camera->GetActorLocation();
	CamDistToPlane = CamToDrone.ProjectOnTo(Camera->GetActorForwardVector()).Size();
	float CamDistToDroneOnDronePlane = CamDistToDroneOnNearPlane / CamDistToNearPlane * CamDistToPlane;

	// convert world location to camera relative location
	WorldLocation = Camera->GetActorLocation() + CamDistToDroneOnDronePlane * WorldDirection;
	FVector RelativeLocation = GameMode->WorldToCameraLocation(WorldLocation);

	HorizontalLocationBound = -RelativeLocation.Y - HorizontalBoundMargin;
	VerticalLocationBound = RelativeLocation.Z - VerticalBoundMargin;

	// rotation bounds
	HorizontalLookBound = HorizontalLocationBound * 70000.f / CamDistToPlane;
	VerticalLookBound = VerticalLocationBound * 70000.f / CamDistToPlane;
}

void APlayerDrone::ConstrainLocation()
{
	FVector RelativeLocation = GameMode->WorldToCameraLocation(GetActorLocation());
	RelativeLocation.Y = FMath::Clamp(RelativeLocation.Y, -HorizontalLocationBound, HorizontalLocationBound);
	RelativeLocation.Z = FMath::Clamp(RelativeLocation.Z, -VerticalLocationBound, VerticalLocationBound);
	SetActorRelativeLocation(RelativeLocation);
}

void APlayerDrone::SetRotation()
{
	if (PlayerController)
	{
		float MouseX;
		float MouseY;
		PlayerController->GetMousePosition(MouseX, MouseY);

		MouseX -= OffsetX;
		MouseY -= OffsetY;
		MouseX = FMath::Clamp(MouseX, 0.f, ScreenX);
		MouseY = FMath::Clamp(MouseY, 0.f, ScreenY);

		FVector LookAtPoint = Camera->GetActorLocation() + 70000.f * Camera->GetActorForwardVector();
		LookAtPoint = GameMode->WorldToCameraLocation(LookAtPoint);
		LookAtPoint.Y = LookAtPoint.Y - HorizontalLookBound + MouseX / ScreenX * 2 * HorizontalLookBound;
		LookAtPoint.Z = -(LookAtPoint.Z - VerticalLookBound + MouseY / ScreenY * 2 * VerticalLookBound);
		LookAtPoint = GameMode->CameraToWorldLocation(LookAtPoint);

		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookAtPoint);
		SetActorRotation(LookAtRotation);
	}
}

void APlayerDrone::AddScore(int32 Val)
{
	Score += Val;

	OnScoreChanged.Broadcast(Score);
}

void APlayerDrone::Reset()
{
	Score = 0;
	Health = MaxHealth;

	OnScoreChanged.Broadcast(Score);
	OnHealthChanged.Broadcast(Health / MaxHealth);
}

void APlayerDrone::Death()
{
	Super::Death();

	TArray<AActor*> ShipParts;
	StarSparrow->GetAttachedActors(ShipParts, true, true);
	for (AActor* ShipPart : ShipParts)
	{
		AStaticMeshActor* ShipPartMesh = Cast<AStaticMeshActor>(ShipPart);
		if (ShipPartMesh)
		{
			UStaticMeshComponent* ShipPartMeshComponent = ShipPartMesh->GetStaticMeshComponent();

			ShipPartMeshComponent->SetCollisionProfileName(FName("PhysicsActor"));
			ShipPartMeshComponent->SetSimulatePhysics(true);
			ShipPartMeshComponent->SetEnableGravity(false);
			ShipPartMeshComponent->AddImpulse(5.f * (ShipPart->GetActorLocation() - GetActorLocation()));
			ShipPartMeshComponent->AddAngularImpulseInDegrees(5.f * (ShipPart->GetActorLocation() - GetActorLocation()));
		}
	}
}

void APlayerDrone::ConstrainViewportAspectRatio()
{
	float GoalX = 16.f * ScreenY / 9.f;
	if (GoalX < ScreenX)
	{
		OffsetX = (ScreenX - GoalX) / 2.f;
		ScreenX = GoalX;
	}
	else
	{
		float GoalY = 9.f * ScreenX / 16.f;
		if (GoalY < ScreenY)
		{
			OffsetY = (ScreenY - GoalY) / 2.f;
			ScreenY = GoalY;
		}
	}
}
