// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NameToStaticMeshRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FNameToStaticMeshRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<UStaticMesh*> staticMesh;

};