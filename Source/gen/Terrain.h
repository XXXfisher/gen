// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "TerrainGenerator.h"
#include "Terrain.generated.h"

UCLASS()
class GEN_API ATerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATerrain();

protected:
	// Called when the game starts or when spawned
	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	UPROPERTY(EditAnywhere, Category = "Terrain")
	int32 Power = 7;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float GridSpacing = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float MaxHeight = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	bool bRegenerate = false; 

	UPROPERTY(EditAnywhere, Instanced, Category = "Terrain")
	UTerrainGenerator* Generator;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> ProcMesh;
	void Generate();
};
