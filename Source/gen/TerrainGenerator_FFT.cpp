// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGenerator_FFT.h"

extern "C" 
{
     #include "kiss_fftr.h"
}

void UTerrainGenerator_FFT::GenerateHeightMap(
    TArray<float>& OutHeight,
    int32 Size,
    int32 Seed
)
{
	if (!FMath::IsPowerOfTwo(Size))
	{
		UE_LOG(LogTemp, Error, TEXT("FFT terrain requires Size to be a power of two."));
		return;
	}

	const int32 N = Size;

	OutHeight.SetNumZeroed(Size * Size);

	FRandomStream Rng(Seed);

	kiss_fftr_cfg CfgForward = kiss_fftr_alloc(N, 0, nullptr, nullptr);
	kiss_fftr_cfg CfgInverse = kiss_fftr_alloc(N, 1, nullptr, nullptr);

	if (!CfgInverse) 
	{ 
		UE_LOG(LogTemp, Error, TEXT("FFT: Failed to allocate inverse FFT config.")); 
	    return; 
	}

	TArray<float> SpatialRow;
	SpatialRow.SetNumZeroed(N);

	TArray<kiss_fft_cpx> FreqRow;
	FreqRow.SetNumZeroed(N / 2 + 1);

	for (int32 y = 0; y < N; ++y) 
	{
		// 1. Generate frequency domain data with random phases and amplitude falloff
		for (int32 k = 0; k < N / 2 + 1; ++k) 
		{
			float f = float(k);

			float BaseAmp = 0.1f;
			float Amp = 1.0f;

			if (k > 0)
			{
				
				Amp += 1.0f / FMath::Pow(f, FrequencyFalloff);
			}

			float phase = Rng.GetFraction() * 2.0f * PI;
			
			FreqRow[k].r = Amp * FMath::Cos(phase);
			FreqRow[k].i = Amp * FMath::Sin(phase);
		}

		// 2. Inverse FFT to get spatial domain row
		kiss_fftri(CfgInverse, FreqRow.GetData(), SpatialRow.GetData());

		
		// 3. Store height values
		for (int32 x = 0; x < N; ++x) 
		{
			//OutHeight[y * N + x] = SpatialRow[x] / N; // Normalize by N

			float h = SpatialRow[x] / float(N); 
			OutHeight[y * N + x] = h;
		}
	}

	free(CfgForward);
	free(CfgInverse);

	// 4. normalize entire height map to [0,1]
	float MinV = OutHeight[0];
	float MaxV = OutHeight[0];

	for (float v : OutHeight)
	{
		MinV = FMath::Min(MinV, v);
		MaxV = FMath::Max(MaxV, v);
	}

	if (FMath::IsNearlyZero(MinV - MaxV))
	{
		UE_LOG(LogTemp, Warning, TEXT("FFT terrain generated a flat heightmap."));
		return;
	}

	const float Den = FMath::Max(1e-6f, MaxV - MinV);

	for (float& v : OutHeight)
	{
		v = (v - MinV) / Den;
	}
}
