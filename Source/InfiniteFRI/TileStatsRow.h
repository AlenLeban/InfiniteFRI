// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TileStatsMappingRow.h"
#include "TileStatsRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FTileStatsRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString tileName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int rotation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, FTileStatsMappingRow> mappings;

	
};
