// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainGenerator.h"
#include "TerrainGenerator_DiamondSquare.generated.h"

/**
 * 
 */
UCLASS(EditInlineNew, Blueprintable)
class GEN_API UTerrainGenerator_DiamondSquare : public UTerrainGenerator
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DiamondSquare")
	float Roughness = 1.0f;

	virtual void GenerateHeightMap(
		TArray<float>& OutHeight,
		int32 Size,
		int32 Seed
	) override;

private:
	void RunDiamondSquare(
		TArray<float>& H,
		int32 Size,
		FRandomStream& Rng
	);

	int32 Idx(int32 Size, int32 X, int32 Y) const {
		return Y * Size + X;
	}


};
