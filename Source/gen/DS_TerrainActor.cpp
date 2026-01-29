#include "DS_TerrainActor.h"
#include "KismetProceduralMeshLibrary.h"


ADS_TerrainActor::ADS_TerrainActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
	SetRootComponent(ProcMesh);

	ProcMesh->bUseAsyncCooking = true;
}

void ADS_TerrainActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 方便你在Details里勾一下就强制刷新
	if (bRegenerate)
	{
		bRegenerate = false;
	}

	Generate();
}

float ADS_TerrainActor::Get(const TArray<float>& H, int32 Size, int32 X, int32 Y) const
{
	return H[Idx(Size, X, Y)];
}

void ADS_TerrainActor::Set(TArray<float>& H, int32 Size, int32 X, int32 Y, float V)
{
	H[Idx(Size, X, Y)] = V;
}

void ADS_TerrainActor::DiamondSquare(TArray<float>& H, int32 Size, FRandomStream& Rng)
{
	// 初始化四角
	Set(H, Size, 0, 0, 0.0f);
	Set(H, Size, Size - 1, 0, 0.0f);
	Set(H, Size, 0, Size - 1, 0.0f);
	Set(H, Size, Size - 1, Size - 1, 0.0f);

	int32 Step = Size - 1;
	float Scale = 1.0f;

	auto RandOffset = [&](float S)
		{
			// [-S, S]
			return (Rng.GetFraction() * 2.0f - 1.0f) * S;
		};

	while (Step > 1)
	{
		const int32 Half = Step / 2;

		// ---- Diamond step ----
		for (int32 y = Half; y < Size - 1; y += Step)
		{
			for (int32 x = Half; x < Size - 1; x += Step)
			{
				const float a = Get(H, Size, x - Half, y - Half);
				const float b = Get(H, Size, x + Half, y - Half);
				const float c = Get(H, Size, x - Half, y + Half);
				const float d = Get(H, Size, x + Half, y + Half);

				const float avg = (a + b + c + d) * 0.25f;
				Set(H, Size, x, y, avg + RandOffset(Scale));
			}
		}

		// ---- Square step ----
		for (int32 y = 0; y < Size; y += Half)
		{
			for (int32 x = (y / Half) % 2 == 0 ? Half : 0; x < Size; x += Step)
			{
				float sum = 0.0f;
				int32 cnt = 0;

				auto AddIfValid = [&](int32 sx, int32 sy)
					{
						if (sx >= 0 && sx < Size && sy >= 0 && sy < Size)
						{
							sum += Get(H, Size, sx, sy);
							cnt++;
						}
					};

				AddIfValid(x - Half, y);
				AddIfValid(x + Half, y);
				AddIfValid(x, y - Half);
				AddIfValid(x, y + Half);

				const float avg = (cnt > 0) ? (sum / (float)cnt) : 0.0f;
				Set(H, Size, x, y, avg + RandOffset(Scale));
			}
		}

		Step = Half;

		// 粗糙度控制：每一层递减随机幅度
		Scale *= FMath::Clamp(Roughness, 0.0f, 1.0f);
	}
}

void ADS_TerrainActor::ThermalErosion(TArray<float>& H, int32 Size, float Talus, int32 Iter)
{
	auto Id = [&](int32 x, int32 y)
		{
			return y * Size + x;
		};

	for (int32 k = 0; k < Iter; k++)
	{
		for (int32 y = 1; y < Size - 1; y++)
		{
			for (int32 x = 1; x < Size - 1; x++)
			{
				float h = H[Id(x, y)];

				float maxDrop = 0.0f;
				int32 nx = x, ny = y;

				// 找最大落差方向
				for (int32 dy = -1; dy <= 1; dy++)
				{
					for (int32 dx = -1; dx <= 1; dx++)
					{
						if (dx == 0 && dy == 0) continue;

						float nh = H[Id(x + dx, y + dy)];
						float drop = h - nh;

						if (drop > maxDrop)
						{
							maxDrop = drop;
							nx = x + dx;
							ny = y + dy;
						}
					}
				}

				// 如果坡度超过 Talus，就移动一点土
				if (maxDrop > Talus)
				{
					float amount = (maxDrop - Talus) * ErosionRate;
					H[Id(x, y)] -= amount;
					H[Id(nx, ny)] += amount;
				}
			}
		}
	}
}


void ADS_TerrainActor::Generate()
{
	const int32 P = FMath::Clamp(Power, 1, 11); // 2^11+1=2049，太大会很慢
	const int32 Size = (1 << P) + 1;           // 2^n + 1

	FRandomStream Rng(Seed);

	TArray<float> Height;
	Height.SetNumZeroed(Size * Size);

	DiamondSquare(Height, Size, Rng);

	ThermalErosion(Height, Size, 0.01f, 200);


	// 归一化到 [0,1] 再映射到 [0, MaxHeight]
	float MinV = Height[0], MaxV = Height[0];
	for (float v : Height) { MinV = FMath::Min(MinV, v); MaxV = FMath::Max(MaxV, v); }
	const float Den = FMath::Max(0.0001f, MaxV - MinV);

	// 顶点
	TArray<FVector> Verts;
	Verts.SetNum(Size * Size);

	for (int32 y = 0; y < Size; y++)
	{
		for (int32 x = 0; x < Size; x++)
		{
			const float n = (Get(Height, Size, x, y) - MinV) / Den; // 0..1
			const float z = n * MaxHeight;
			Verts[Idx(Size, x, y)] = FVector(x * GridSpacing, y * GridSpacing, z);
		}
	}

	// 三角形索引
	TArray<int32> Tris;
	Tris.Reserve((Size - 1) * (Size - 1) * 6);

	for (int32 y = 0; y < Size - 1; y++)
	{
		for (int32 x = 0; x < Size - 1; x++)
		{
			const int32 i0 = Idx(Size, x, y);
			const int32 i1 = Idx(Size, x + 1, y);
			const int32 i2 = Idx(Size, x, y + 1);
			const int32 i3 = Idx(Size, x + 1, y + 1);

			// 两个三角形（注意顺序影响法线方向）
			Tris.Add(i0); Tris.Add(i2); Tris.Add(i1);
			Tris.Add(i1); Tris.Add(i2); Tris.Add(i3);
		}
	}

	// 法线（简单：用 UE 的函数算）
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;
	TArray<FLinearColor> Colors;

	UV0.SetNum(Size * Size);
	for (int32 y = 0; y < Size; y++)
	{
		for (int32 x = 0; x < Size; x++)
		{
			UV0[Idx(Size, x, y)] = FVector2D((float)x / (Size - 1), (float)y / (Size - 1));
		}
	}

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Verts, Tris, UV0, Normals, Tangents);


	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection_LinearColor(
		0,
		Verts,
		Tris,
		Normals,
		UV0,
		Colors,
		Tangents,
		true // collision
	);

	ProcMesh->ContainsPhysicsTriMeshData(true);
}
