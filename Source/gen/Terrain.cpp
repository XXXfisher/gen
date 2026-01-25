// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrain.h"
#include "TerrainGenerator_DiamondSquare.h"
#include "TerrainGenerator_FFT.h"


// Sets default values
ATerrain::ATerrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
	SetRootComponent(ProcMesh);
	ProcMesh->bUseAsyncCooking = true;

}

void ATerrain::OnConstruction(const FTransform& Transform)
{
	if (bRegenerate)
	{
		bRegenerate = false;
		Generate();
	}
}

void ATerrain::Generate()
{
	if (!Generator) {
		UE_LOG(LogTemp, Warning, TEXT("No Terrain Generator assigned!"));
		return;
	}

	int32 Size = 0;

	// Diamond-Square 使用 (2^Power + 1)
	if (Generator->IsA(UTerrainGenerator_DiamondSquare::StaticClass()))
	{
		Size = (1 << Power) + 1;
	}
	// FFT 使用 2^Power
	else if (Generator->IsA(UTerrainGenerator_FFT::StaticClass()))
	{
		Size = (1 << Power);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unknown terrain generator type!"));
		return;
	}

	TArray<float> Height;
	Generator->GenerateHeightMap(Height, Size, Seed);


	// Build mesh data
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;

	for (int32 y = 0; y < Size; y++)
	{
		for (int32 x = 0; x < Size; x++)
		{
			float h = Height[y * Size + x];
			Vertices.Add(FVector(x * GridSpacing, y * GridSpacing, h * MaxHeight));
			UVs.Add(FVector2D((float)x / (Size - 1), (float)y / (Size - 1)));
		}
	}

	for (int32 y = 0; y < Size - 1; y++)
	{
		for (int32 x = 0; x < Size - 1; x++)
		{
			int32 i0 = y * Size + x;
			int32 i1 = y * Size + (x + 1);
			int32 i2 = (y + 1) * Size + x;
			int32 i3 = (y + 1) * Size + (x + 1);
			Triangles.Add(i0);
			Triangles.Add(i2);
			Triangles.Add(i1);
			Triangles.Add(i1);
			Triangles.Add(i2);
			Triangles.Add(i3);
		}
	}

	Normals.SetNumZeroed(Vertices.Num());
	for (int32 i = 0; i < Triangles.Num(); i += 3)
	{
		FVector a = Vertices[Triangles[i]];
		FVector b = Vertices[Triangles[i + 1]];
		FVector c = Vertices[Triangles[i + 2]];
		FVector n = FVector::CrossProduct(b - a, c - a).GetSafeNormal();

		Normals[Triangles[i]] += n;
		Normals[Triangles[i + 1]] += n;
		Normals[Triangles[i + 2]] += n;
	}
	for (FVector& n : Normals)
	{
		n.Normalize();
	}
	ProcMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, {}, Tangents, true);
};