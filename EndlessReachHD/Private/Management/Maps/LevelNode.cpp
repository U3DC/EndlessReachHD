// © 2012 - 2019 Soverance Studios
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
#include "EndlessReachHDGameMode.h"
#include "EndlessReachHDPawn.h"
#include "LevelNode.h"

// Sets default values
ALevelNode::ALevelNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curve(TEXT("/Game/Curves/WormholeCurve.WormholeCurve"));
	WormholeCurve = Curve.Object;
	WormholeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("WormholeTimeline"));
	InterpFunction.BindUFunction(this, FName{ TEXT("TimelineFloatReturn") });
	
	// configure Node Entry Radius
	NodeEntryRadius = CreateDefaultSubobject<USphereComponent>(TEXT("NodeEntryRadius"));
	NodeEntryRadius->SetupAttachment(RootComponent);
	NodeEntryRadius->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	NodeEntryRadius->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	NodeEntryRadius->OnComponentBeginOverlap.AddDynamic(this, &ALevelNode::OnOverlap);
	NodeEntryRadius->SetSphereRadius(250);
	NodeEntryRadius->SetSimulatePhysics(true);
	NodeEntryRadius->BodyInstance.bLockZTranslation = true;
	RootComponent = NodeEntryRadius;

	// Portal Visual Effect
	static ConstructorHelpers::FObjectFinder<UParticleSystem> NodePortalParticleObject(TEXT("/Game/VectorFields/Particles/P_Gateway.P_Gateway"));
	P_NodePortalFX = NodePortalParticleObject.Object;
	NodePortalFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("NodePortalFX"));
	NodePortalFX->SetupAttachment(RootComponent);
	NodePortalFX->Template = P_NodePortalFX;
	NodePortalFX->SetRelativeRotation(FRotator(0, 0, 0));
	NodePortalFX->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
	NodePortalFX->bAutoActivate = false;

	// Teleport Visual Effect
	static ConstructorHelpers::FObjectFinder<UParticleSystem> NodeTeleportParticleObject(TEXT("/Game/VectorFields/Particles/P_Wormhole.P_Wormhole"));
	P_NodeTeleportFX = NodeTeleportParticleObject.Object;
	NodeTeleportFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("NodeTeleportFX"));
	NodeTeleportFX->SetupAttachment(RootComponent);
	NodeTeleportFX->Template = P_NodeTeleportFX;
	NodeTeleportFX->SetWorldRotation(FRotator(0, 0, 90));
	NodeTeleportFX->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
	NodeTeleportFX->bAutoActivate = false;
}

// Called when the game starts or when spawned
void ALevelNode::BeginPlay()
{
	Super::BeginPlay();
	
	// if the level is unlocked, then activate the portal to show this node on the map
	if (!LevelConfig.bIsLocked)
	{
		NodePortalFX->Activate();  // activate portal fx
		WormholeStartLocation = NodeTeleportFX->GetComponentLocation();  // store wormhole start location 
	}
}

// Called every frame
void ALevelNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelNode::OnOverlap(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// exclude overlap functions if this level is locked
	if (!LevelConfig.bIsLocked)
	{
		if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
		{
			AEndlessReachHDPawn* HitPlayer = Cast<AEndlessReachHDPawn>(OtherActor);  // Check if hit actor is the player

			// Proceed with functions if you hit the player
			if (HitPlayer)
			{
				Player = HitPlayer;  // store the player reference for later use

				// Make sure it's actually the player ship that entered this zone, and not the player's bullets
				if (OtherComp == HitPlayer->ShipMeshComponent)
				{
					// if the level is unlocked, allow it to proceed
					if (!LevelConfig.bIsLocked)
					{
						// ensure this node is not currently in use
						if (!bInUse)
						{
							bInUse = true;  // flag in use
							Player->bCanMove = false;  // disable player movement input

							NodePortalFX->Deactivate();  // deactivate portal FX
							NodeTeleportFX->Activate();  // activate wormhole FX

							WormholeTimeline->AddInterpFloat(WormholeCurve, InterpFunction, FName{ TEXT("Float") });
							WormholeTimeline->PlayFromStart();  // play the wormhole teleport animation - I NEED TO REPLACE THIS BULLSHIT WITH A SEQUENCER MOVIE

							// Do the stomp blast after the timeline completes
							FTimerHandle LoadTimer;
							GetWorldTimerManager().SetTimer(LoadTimer, this, &ALevelNode::WormholeUnload, 3.0f, false);
						}
					}
				}
			}
		}
	}	
}

// Wormhole Timeline
void ALevelNode::TimelineFloatReturn(float val)
{
	FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();  // Store Camera Location in World Space
	float NewX = FMath::Lerp(FMath::FloorToInt(WormholeStartLocation.X), FMath::FloorToInt(CameraLocation.X), val);  // create new X value
	float NewY = FMath::Lerp(FMath::FloorToInt(WormholeStartLocation.Y), FMath::FloorToInt(CameraLocation.Y), val);  // create new Y value
	float NewZ = FMath::Lerp(FMath::FloorToInt(WormholeStartLocation.Z), FMath::FloorToInt(CameraLocation.Z), val);  // create new Z value
	// lerp from the wormhole's starting Z location to camera's current Z location
	NodeTeleportFX->SetWorldLocation(FVector(NewX, NewY, NewZ));
}

// Wormhole Unload
void ALevelNode::WormholeUnload()
{
	AEndlessReachHDGameMode* GameMode = Cast<AEndlessReachHDGameMode>(GetWorld()->GetAuthGameMode());  // collect game mode reference
	GameMode->UnloadMap(GameMode->GetMapName(GameMode->CurrentMap), GameMode->CurrentMap);  // unload the current map
	GameMode->LevelNodes[GameMode->CurrentMap]->LevelConfig.bIsLocked = false;  // unlock the current level, since it's been unloaded
	GameMode->LevelNodes[GameMode->CurrentMap]->NodePortalFX->Activate();  // activate node portal fx to show the current level's node on the map
	FTimerHandle LoadTimer;
	GetWorldTimerManager().SetTimer(LoadTimer, this, &ALevelNode::WormholeLoad, 4.0f, false);
}

// Wormhole Load
void ALevelNode::WormholeLoad()
{
	AEndlessReachHDGameMode* GameMode = Cast<AEndlessReachHDGameMode>(GetWorld()->GetAuthGameMode());  // collect game mode reference
	GameMode->ReloadMap(LevelConfig.MapName, LevelConfig.NodeNumber);  // Load the map which corresponds to this node
	GameMode->CurrentMap = LevelConfig.NodeNumber;  // set the current map value in Game Mode
	LevelConfig.bIsLocked = true;  // lock this level while it's loaded
	bInUse = false;  // flag this node as no longer in use
	NodeTeleportFX->Deactivate();  // disable teleport fx
	Player->bCanMove = true;  // re-enable movement
}