// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGenerator_DiamondSquare.h"

void UTerrainGenerator_DiamondSquare::GenerateHeightMap(
	TArray<float>& OutHeight,
	int32 Size,
	int32 Seed
)
{
	OutHeight.SetNumZeroed(Size * Size);
	FRandomStream Rng(Seed);
	RunDiamondSquare(OutHeight, Size, Rng);

	float minV = OutHeight[0];
	float maxV = OutHeight[0];
	
	for (float v : OutHeight) 
	{
		minV = FMath::Min(minV, v);
		maxV = FMath::Max(maxV, v);

	}

	float Den = FMath::Max(0.0001f, maxV - minV);

	for (float& v : OutHeight) {
		v = (v - minV) / Den;
	}
}

void UTerrainGenerator_DiamondSquare::RunDiamondSquare(
	TArray<float>& H,
	int32 Size,
	FRandomStream& Rng
)
{
	const int32 MaxIndex = Size - 1;

	H[Idx(Size, 0, 0)] = 0.5f;
	H[Idx(Size, MaxIndex, 0)] = 0.5f;
	H[Idx(Size, 0, MaxIndex)] = 0.5f;
	H[Idx(Size, MaxIndex, MaxIndex)] = 0.5f;

	int32 Step = MaxIndex;
	float Scale = Roughness;

	while (Step > 1) {
		int32 HalfStep = Step / 2;

		// Diamond step
		for (int32 y = HalfStep; y < MaxIndex; y += Step) {
			for (int32 x = HalfStep; x < MaxIndex; x += Step) {
				float a = H[Idx(Size, x - HalfStep, y - HalfStep)];
				float b = H[Idx(Size, x + HalfStep, y - HalfStep)];
				float c = H[Idx(Size, x - HalfStep, y + HalfStep)];
				float d = H[Idx(Size, x + HalfStep, y + HalfStep)];
				
				float avg = (a + b + c + d) * 0.25f;
				float offset = (Rng.GetFraction() * 2.0f - 1.0f) * Scale;

				H[Idx(Size, x, y)] = avg + offset;
			}
		}

		// Square step
		for (int32 y = 0; y <= MaxIndex; y += HalfStep) {

			int32 shift = (y / HalfStep) % 2 == 0 ? HalfStep : 0;

			for (int32 x = shift; x <= MaxIndex; x += Step) 
			{
				float sum = 0.0f;
				int32 count = 0;

				if (x - HalfStep >= 0) {
					sum += H[Idx(Size, x - HalfStep, y)];
					count++;
				}
				if (x + HalfStep <= MaxIndex) {
					sum += H[Idx(Size, x + HalfStep, y)];
					count++;
				}
				if (y - HalfStep >= 0) {
					sum += H[Idx(Size, x, y - HalfStep)];
					count++;
				}
				if (y + HalfStep <= MaxIndex) {
					sum += H[Idx(Size, x, y + HalfStep)];
					count++;
				}

				float avg = sum / FMath::Max(1, count);
				float offset = (Rng.GetFraction() * 2.0f - 1.0f) * Scale;

				H[Idx(Size, x, y)] = avg + offset;
			}
		}

		Step /= 2;
		Scale *= Roughness;
	}
}
