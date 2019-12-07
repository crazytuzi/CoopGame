// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerupActor.h"
#include <UnrealNetwork.h>

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PowerupInterval = 0.f;
	TotalNrOfTicks = 0;

	bIsPowerupActive = false;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASPowerupActor::BeginPlay()
{
	Super::BeginPlay();

	TicksProcessed = 0;
}

void ASPowerupActor::OnTickPowerup()
{
	++TicksProcessed;

	OnPowerupTicked();

	if (TotalNrOfTicks <= TicksProcessed)
	{
		OnExpired();

		bIsPowerupActive = false;

		// Delete timer
		GetWorldTimerManager().ClearTimer(TimeHandle_PowerupTick);
	}
}

void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerupActor::ActivatePowerup(AActor* ActiveFor)
{
	OnActivated(ActiveFor);

	bIsPowerupActive = true;

	if (PowerupInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimeHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval,
		                                true);
	}
	else
	{
		OnTickPowerup();
	}
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}
