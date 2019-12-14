// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include <UnrealNetwork.h>

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons")
	,
	DebugWeaponDrawing
	,
	TEXT("Draw Debug Lines for Weapons")
	,
	ECVF_Cheat
);

// Sets default values
ASWeapon::ASWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";

	BaseDamage = 20.f;

	BulletSpread = 2.f;

	RateOfFire = 600.f;

	SetReplicates(true);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

void ASWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}
	AActor* MyOwner = GetOwner();
	if (MyOwner != nullptr)
	{
		FVector EyeLoction;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLoction, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		// Bullet Spread
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

		FVector TraceEnd = EyeLoction + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FVector TraceEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLoction, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit,
			                                   MyOwner->GetInstigatorController(),
			                                   this, DamageType);

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TraceEndPoint = Hit.ImpactPoint;

			HitScanTrace.SurfaceType = SurfaceType;
		}

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLoction, TraceEnd, FColor::White, false, 1.f, 0, 1.f);
		}

		PlayFireEffects(TraceEndPoint);

		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TraceEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

void ASWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(0.f, LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true,
	                                FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffects(FVector TraceEndPoint)
{
	if (MuzzleEffect != nullptr)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TraceEffect != nullptr)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TraceComp = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), TraceEffect, MuzzleLocation);
		if (TraceComp != nullptr)
		{
			TraceComp->SetVectorParameter(TracerTargetName, TraceEndPoint);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner != nullptr)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC != nullptr)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect != nullptr)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint,
		                                         ShotDirection.Rotation());
	}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
