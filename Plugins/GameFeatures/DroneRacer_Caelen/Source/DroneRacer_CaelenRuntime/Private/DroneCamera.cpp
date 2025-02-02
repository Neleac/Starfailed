#include "DroneCamera.h"

#include "Kismet/GameplayStatics.h"

UDroneCamera::UDroneCamera()
{
	PrimaryComponentTick.bCanEverTick = true;

	prevPos = FVector::ZeroVector;
}

void UDroneCamera::BeginPlay()
{
	Super::BeginPlay();

	prevPos = GetOwner()->GetActorLocation();
}

void UDroneCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector currPos = GetOwner()->GetActorLocation();
	velocity = (currPos - prevPos) / DeltaTime;
	prevPos = currPos;
}

FVector UDroneCamera::GetVelocity()
{
	return velocity;
}
