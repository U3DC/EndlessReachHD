// 2012 - 2019 Soverance Studios
// https://soverance.com

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "Pickups/PickupMaster.h"
#include "FuelCell.generated.h"

UCLASS()
class ENDLESSREACHHD_API AFuelCell : public APickupMaster
{
	GENERATED_BODY()
	
	/* The mesh component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* FuelMeshComponent;

	/* The orb material */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* FuelColor;

public:
	// Sets default values for this actor's properties
	AFuelCell();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// On Player Collision
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void CollideWithPlayer();

	UPROPERTY(Category = Gameplay, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UParticleSystemComponent* FuelPickupFX;
	UParticleSystem* P_FuelPickupFX;

	/** Orb Pickup Sound */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	USoundCue* S_FuelPickup;
	UAudioComponent* FuelPickupSound;
	
	// Reconfigure Physics
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void ReconfigurePhysics();
};
