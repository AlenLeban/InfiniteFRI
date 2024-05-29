// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TileStatsCountRow.h"
#include "TileStatsMappingRow.generated.h"


/**
 * 
 */
USTRUCT(BlueprintType)
struct FTileStatsMappingRow
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, float> dirToCountMap;

};
