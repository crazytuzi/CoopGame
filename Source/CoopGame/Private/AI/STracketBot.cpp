// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/STracketBot.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include <GameFramework/Character.h>
#include <Kismet/GameplayStatics.h>
#include <DrawDebugHelpers.h>
#include "SHealthComponent.h"

// Sets default values
ASTracketBot::ASTracketBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealComp"));
	HealComp->OnHealthChanged.AddDynamic(this, &ASTracketBot::HandleTakeDamage);

	bUseVelocityChange = true;

	MovementForce = 1000.f;

	RequireDistanceToTarget = 100.f;
}

// Called when the game starts or when spawned
void ASTracketBot::BeginPlay()
{
	Super::BeginPlay();

	// Find initial move-to
	NextPathPoint = GetNextPathPoint();
}

void ASTracketBot::HandleTakeDamage(const USHealthComponent* HealthComp, float Health, float HealthDelta,
                                    const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// Explode on hitpoints = 0

	// @TODO: Pulse the material on hit

	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst != nullptr)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}


	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
}

FVector ASTracketBot::GetNextPathPoint()
{
	// Hack, to get player location
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if (NavPath != nullptr)
	{
		if (NavPath->PathPoints.Num() > 0)
		{
			// Return next point in the path
			return NavPath->PathPoints[1];
		}
	}

	// Failed to find path
	return GetActorLocation();
}

// Called every frame
void ASTracketBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

	if (DistanceToTarget <= RequireDistanceToTarget)
	{
		NextPathPoint = GetNextPathPoint();

		DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
	}
	else
	{
		// Keep moving towards next target
		FVector ForceDirection = NextPathPoint - GetActorLocation();
		ForceDirection.Normalize();

		ForceDirection *= MovementForce;

		MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32.f,
		                          FColor::Yellow, false, 0.f, 1.f);
	}

	DrawDebugSphere(GetWorld(), NextPathPoint, 20.f, 12.f, FColor::Yellow, false, 0.f, 1.f);
}
