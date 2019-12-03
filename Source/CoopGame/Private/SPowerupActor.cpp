// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerupActor.h"

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PowerupInterval = 0.f;
	TotalNrOfTicks = 0;
}

// Called when the game starts or when spawned
void ASPowerupActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASPowerupActor::OnTickPowerup()
{
	++TicksProcessed;

	OnPowerupTicked();

	if (TotalNrOfTicks >= TicksProcessed)
	{
		OnExpired();

		// Delete timer
		GetWorldTimerManager().ClearTimer(TimeHandle_PowerupTick);
	}
}

void ASPowerupActor::ActivatePowerup()
{
	OnActivated();

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
