#pragma once

#include "CoreMinimal.h"
#include "Drone.h"
#include "PlayerDrone.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedDelegate, float, HealthPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChangedDelegate, int32, ScoreValue);

class UInputAction;
class UInputMappingContext;
class USoundBase;
class APlayerController;

UCLASS()
class DRONERACER_CAELENRUNTIME_API APlayerDrone : public ADrone
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	bool DevMode;

	// viewport variables
	int32 ScreenX;
	int32 ScreenY;
	float OffsetX;	// width of left/right black border, if any
	float OffsetY;	// height of top/bottom black border, if any

	void ConstrainViewportAspectRatio(); // constrain to 16:9
	
	// rotation bound
	float HorizontalLookBound;
	float VerticalLookBound;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<AActor> StarSparrow;

	int32 Score;

	float MouseInactive;

	virtual void SetLocationBounds() override;

	UFUNCTION(BlueprintCallable)
	void Reset();

	virtual void Death() override;

	UPROPERTY(EditAnywhere)
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere)
	UInputAction* WeaponFireAction;

	UPROPERTY(EditAnywhere)
	USoundBase* AsteroidImpactSound;

public:
	APlayerDrone();
	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void AddScore(int32 Val);

	UFUNCTION()
	virtual void BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(BlueprintAssignable)
	FOnHealthChangedDelegate OnHealthChanged;
	UPROPERTY(BlueprintAssignable)
	FOnScoreChangedDelegate OnScoreChanged;

protected:
	APlayerController* PlayerController;

	virtual void BeginPlay() override;

	void ConstrainLocation();

	void SetRotation();

	void Input_Move(const FInputActionValue& Value);

	void Input_LookMouse(const FInputActionValue& Value);

	void Input_WeaponFire(const FInputActionValue& Value);
};
