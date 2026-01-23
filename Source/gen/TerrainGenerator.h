#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TerrainGenerator.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class GEN_API UTerrainGenerator : public UObject
{
    GENERATED_BODY()

public:

    virtual void GenerateHeightMap(
        TArray<float>& OutHeight,
        int32 Size,
        int32 Seed
    ) PURE_VIRTUAL(UTerrainGenerator::GenerateHeightMap, );
};
