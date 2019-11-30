// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/STracketBot.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include <GameFramework/Character.h>
#include <Kismet/GameplayStatics.h>

// Sets default values
ASTracketBot::ASTracketBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);

	RootComponent = MeshComp;
}

// Called when the game starts or when spawned
void ASTracketBot::BeginPlay()
{
	Super::BeginPlay();
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
}
