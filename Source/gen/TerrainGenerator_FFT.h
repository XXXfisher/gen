// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainGenerator.h"
#include "TerrainGenerator_FFT.generated.h"

/**
 * 
 */
UCLASS(EditInlineNew, Blueprintable)
class GEN_API UTerrainGenerator_FFT : public UTerrainGenerator
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFT")
	float FrequencyFalloff = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFT")
	int32 Octaves = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFT")
	float Gain = 50.0f;


	virtual void GenerateHeightMap(
		TArray<float>& OutHeight,
		int32 Size,
		int32 Seed
	) override;
	
};
