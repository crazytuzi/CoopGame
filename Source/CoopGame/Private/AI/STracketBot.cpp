// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/STracketBot.h"
#include "Components/StaticMeshComponent.h"

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

// Called every frame
void ASTracketBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
