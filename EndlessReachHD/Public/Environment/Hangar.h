// © 2014 - 2017 Soverance Studios
// http://www.soverance.com

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

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hangar.generated.h"

UCLASS()
class ENDLESSREACHHD_API AHangar : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHangar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** The root scene component */
	UPROPERTY(Category = Asteroids, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* Root;

	// Hangar Entry Radius Collider
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components)
	UBoxComponent* HangarEntryRadius;
	
	/** Function to handle the object overlapping something */
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/* Whether the player is currently docked in the hangar */
	UPROPERTY(Category = Docking, EditAnywhere, BlueprintReadWrite)
	bool bIsDocked;
};
