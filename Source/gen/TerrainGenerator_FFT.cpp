// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGenerator_FFT.h"

extern "C" 
{
     #include "kiss_fft.h"
}

static FORCEINLINE int32 WrapIdx(int32 i, int32 N)
{
	i %= N;
	return (i < 0) ? (i + N) : i;
}

static FORCEINLINE int32 Idx2D(int32 x, int32 y, int32 N)
{
	return y * N + x;
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

	// 0. KissFFT inverse config (1D, complex)
	kiss_fft_cfg CfgInv = kiss_fft_alloc(N, /*inverse=*/1, nullptr, nullptr);
	if (!CfgInv)
	{
		UE_LOG(LogTemp, Error, TEXT("FFT: Failed to allocate inverse FFT config."));
		return;
	}

	FRandomStream Rng(Seed);

	TArray<kiss_fft_cpx> Spectrum;
	Spectrum.SetNumZeroed(N * N);
	
	auto SetCpx = [&](int32 kx, int32 ky, float re, float im) {
		Spectrum[Idx2D(kx, ky, N)].r = re;
		Spectrum[Idx2D(kx, ky, N)].i = im;
		};

	auto GetCpx = [&](int32 kx, int32 ky) -> kiss_fft_cpx
		{
			return Spectrum[Idx2D(kx, ky, N)];
		};

	// Fill only a half-plane and mirror using conjugate:
	for (int32 ky = 0; ky < N; ++ky) 
	{
		// 1. Generate frequency domain data with random phases and amplitude falloff
		for (int32 kx = 0; kx < N / 2 + 1; ++kx) 
		{
			// only generate when (ky < N/2) or (ky == N/2 and kx <= N/2) and then mirror to (-kx, -ky).
			const bool bCanonical =
				(ky < (N / 2)) || (ky == (N / 2) && kx <= (N / 2));

			if (!bCanonical) continue;

			const int32 sx = (kx <= N / 2) ? kx : kx - N;
			const int32 sy = (ky <= N / 2) ? ky : ky - N;

			const float r = FMath::Sqrt(float(sx * sx + sy * sy));

			if (sx == 0 && sy == 0)
			{
				SetCpx(kx, ky, 0.0f, 0.0f);
			}
			else 
			{
				// Amplitude falloff by radial frequency
				float AmpSum = 0.0f;

				for (int32 o = 0; o < Octaves; ++o)
				{
					float rr = r * FMath::Pow(2.0f, o);

					float Amp = 1.0f / FMath::Pow(FMath::Max(1.0f, rr), FrequencyFalloff);

					float phase = Rng.GetFraction() * 2 * PI;

					AmpSum += Amp * FMath::Cos(phase); 
				}


				const float Amp = AmpSum;


				const float phase = Rng. GetFraction() * 2.0f * PI;
				const float re = Amp * FMath::Cos(phase);
				const float im = Amp * FMath::Sin(phase);

				SetCpx(kx, ky, re, im);
			}

			// Mirror index (-kx, -ky) -> (N-kx mod N, N-ky mod N)
			const int32  mkx = WrapIdx(N - kx, N);
			const int32  mky = WrapIdx(N - ky, N);

			const kiss_fft_cpx v = GetCpx(kx, ky);
			SetCpx(mkx, mky, v.r, -v.i);

			
		}

	}

	//  2. 2D IFFT via separability: IFFT rows then IFFT columns

	// Row-wise IFFT
	TArray<kiss_fft_cpx> RowIFFT;
	RowIFFT.SetNumZeroed(N * N);

	for (int32 y = 0; y < N; ++y)
	{
		kiss_fft(CfgInv,
			&Spectrum[y * N],
			&RowIFFT[y * N]);
	}

	// Column-wise IFFT
	TArray<kiss_fft_cpx> ColIn, ColOut;
	ColIn.SetNumZeroed(N);
	ColOut.SetNumZeroed(N);

	TArray<kiss_fft_cpx> Spatial;
	Spatial.SetNumZeroed(N * N);

	for (int32 x = 0; x < N; ++x)
	{
		// gather column
		for (int32 y = 0; y < N; ++y)
		{
			ColIn[y] = RowIFFT[Idx2D(x, y, N)];
		}

		// inverse FFT on that column
		kiss_fft(CfgInv, ColIn.GetData(), ColOut.GetData());

		// scatter back
		for (int32 y = 0; y < N; ++y)
		{
			Spatial[Idx2D(x, y, N)] =ColOut[y];
		}
	}

	// 3.Convert to real height + normalize
	const float Scale = 1.0f / float(N * N);

	for (int32 i = 0; i < N * N; ++i)
	{
		OutHeight[i] = Spatial[i].r * Scale * Gain;
	}


	// 不做 Min/Max 归一化
	// 只做一个可控的增益
	for (float& v : OutHeight)
	{
		v *= Gain;
	}


	free(CfgInv);

	// 0--1
	/*float MinV = OutHeight[0];
	float MaxV = OutHeight[0];

	for (float v : OutHeight)
	{
		MinV = FMath::Min(MinV, v);
		MaxV = FMath::Max(MaxV, v);

	}

	if (FMath::IsNearlyZero(MaxV - MinV))
	{
		UE_LOG(LogTemp, Warning, TEXT("FFT terrain generated a flat heightmap."));
		return;
	}

	const float Den = FMath::Max(1e-6f, MaxV - MinV);
	for (float& v : OutHeight)
	{
		v = (v - MinV) / Den;
	}*/
}
