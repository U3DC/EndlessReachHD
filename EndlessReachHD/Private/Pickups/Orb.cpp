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

#include "EndlessReachHD.h"
#include "Orb.h"

// Sets default values
AOrb::AOrb()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bPickedUp = false;

	// Orb Materials	
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> OrbInst(TEXT("/Game/ShipScout_Upgrades/Materials/Ammo_Materials/OrbMaterial.OrbMaterial"));
	OrbColor = OrbInst.Object;

	// Orb Body
	static ConstructorHelpers::FObjectFinder<UStaticMesh> OrbMesh(TEXT("/Game/ShipScout_Upgrades/Ammo/Cannonball.Cannonball"));
	OrbMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrbBody"));
	RootComponent = OrbMeshComponent;
	OrbMeshComponent->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	OrbMeshComponent->OnComponentHit.AddDynamic(this, &AOrb::OnHit);  // set up a notification for when this component hits something
	OrbMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AOrb::OnOverlap);
	OrbMeshComponent->SetStaticMesh(OrbMesh.Object);
	OrbMeshComponent->SetMaterial(0, OrbColor);
	OrbMeshComponent->SetRelativeRotation(FRotator(0, 0, 0));
	OrbMeshComponent->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
	OrbMeshComponent->SetSimulatePhysics(true);
	OrbMeshComponent->BodyInstance.bLockZTranslation = true;
	OrbMeshComponent->BodyInstance.bLockXRotation = true;
	OrbMeshComponent->BodyInstance.bLockYRotation = true;
	OrbMeshComponent->SetLinearDamping(10.0f);  // Increase linear damping to slow down translation
	OrbMeshComponent->SetAngularDamping(10.0f);  // Increase angular damping to slow down rotation	

	static ConstructorHelpers::FObjectFinder<USoundCue> OrbPickupAudio(TEXT("/Game/Audio/Pickups/Pickup_Orb_Cue.Pickup_Orb_Cue"));
	S_OrbPickup = OrbPickupAudio.Object;
	OrbPickupSound = CreateDefaultSubobject<UAudioComponent>(TEXT("OrbPickupSound"));
	OrbPickupSound->SetupAttachment(RootComponent);
	OrbPickupSound->Sound = S_OrbPickup;
	OrbPickupSound->bAutoActivate = false;
}

// Called when the game starts or when spawned
void AOrb::BeginPlay()
{
	Super::BeginPlay();

	OnPlayerCollision.AddDynamic(this, &AOrb::CollideWithPlayer);  // bind player collision function
	
	FVector Velocity = FMath::VRand();  // get random vector
	OrbMeshComponent->AddImpulseAtLocation(Velocity, GetActorLocation());  // spread the orbs around randomly  - (PHYSICS) MOVEMENT METHOD

	// reconfigure physics so the orb overlaps instead of colliding
	FTimerHandle PhysicsResetDelay;
	GetWorldTimerManager().SetTimer(PhysicsResetDelay, this, &AOrb::ReconfigurePhysics, 0.25f, false);
}

// Called every frame
void AOrb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Reconfigure Physics
void AOrb::ReconfigurePhysics()
{
	//OrbMeshComponent->SetSimulatePhysics(false);
	OrbMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

// Collide With Player
void AOrb::CollideWithPlayer()
{
	if (Player)
	{
		Player->OrbCount = Player->OrbCount++;  // increase orb count
		Player->OrbCount = Player->OrbCount + 1000;  // increase orb count massively for debug purposes
		OrbPickupSound->Play();  // play orb pickup sound
		OrbMeshComponent->SetVisibility(false);  // hide orb from player view

		// delay destruction so that audio can finish playing
		FTimerHandle DestroyDelay;
		GetWorldTimerManager().SetTimer(DestroyDelay, this, &AOrb::StartDestruction, 2.0f, false);
	}	
}