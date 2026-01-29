#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "DS_TerrainActor.generated.h"

UCLASS()
class GEN_API ADS_TerrainActor : public AActor
{
	GENERATED_BODY()

public:
	ADS_TerrainActor();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	UPROPERTY(EditAnywhere, Category = "DiamondSquare")
	int32 Power = 8; // 2^8 + 1 = 257

	UPROPERTY(EditAnywhere, Category = "DiamondSquare")
	float GridSpacing = 100.0f;

	UPROPERTY(EditAnywhere, Category = "DiamondSquare")
	float MaxHeight = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "DiamondSquare")
	float Roughness = 0.5f; // 0~1, 越大越“崎岖”

	UPROPERTY(EditAnywhere, Category = "DiamondSquare")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, Category = "DiamondSquare")
	bool bRegenerate = false; // 勾一下强制重建（方便）

	UPROPERTY(EditAnywhere, Category = "Erosion")
	float ErosionRate = 0.5f;


private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> ProcMesh;

	void Generate();

	void DiamondSquare(TArray<float>& H, int32 Size, FRandomStream& Rng);
	void ThermalErosion(TArray<float>& H, int32 Size, float Talus, int32 Iter);
	float Get(const TArray<float>& H, int32 Size, int32 X, int32 Y) const;
	void Set(TArray<float>& H, int32 Size, int32 X, int32 Y, float V);
	int32 Idx(int32 Size, int32 X, int32 Y) const { return Y * Size + X; }
};
