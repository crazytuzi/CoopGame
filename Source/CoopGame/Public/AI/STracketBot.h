// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STracketBot.generated.h"

class USHealthComponent;
class UMaterialInstanceDynamic;

UCLASS()

class COOPGAME_API ASTracketBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTracketBot();

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealComp;

	UFUNCTION()
	void HandleTakeDamage(const class USHealthComponent* HealthComp, float Health,
	                      float HealthDelta, const class UDamageType* DamageType,
	                      class AController* InstigatedBy, AActor* DamageCauser);

	FVector GetNextPathPoint();

	// Next point in navigation path
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TracketBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TracketBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TracketBot")
	float RequireDistanceToTarget;

	// Dynamic material to pulse on damage
	UMaterialInstanceDynamic* MatInst;

public:
	// Called every frame
	void Tick(float DeltaTime) override;
};
