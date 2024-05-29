// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TileStatsCountRow.generated.h"

USTRUCT(BlueprintType)
struct FTileStatsCountRow
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, int> tileToCountMap;

};