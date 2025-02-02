#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneCamera.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DRONERACER_CAELENRUNTIME_API UDroneCamera : public UActorComponent
{
	GENERATED_BODY()

	FVector prevPos;
	FVector velocity;

public:	
	UDroneCamera();

	FVector GetVelocity();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
};
