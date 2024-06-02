// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GridCell.generated.h"

class AFRIGenerator;
class AStaticMeshActor;

/**
 * 
 */
UCLASS()
class INFINITEFRI_API UGridCell : public UObject
{
	GENERATED_BODY()

public:


	UPROPERTY()
	TArray<int> states;
	
	UPROPERTY()
	FIntVector location;

	UPROPERTY()
	AFRIGenerator* generatorRef;

	UPROPERTY()
	int checkIndex = 0; 

	UPROPERTY()
	bool isCollapsed = false;
	
	UPROPERTY()
	bool areStatesDirty = true;

	UPROPERTY()
	float entropy = 99999;

	UPROPERTY()
	AStaticMeshActor* staticMeshActorRef = nullptr;

public:

	static bool gridCellPredicate(UGridCell& first, UGridCell& second)
	{
		return first.GetEntropy() < second.GetEntropy();
	};

	UFUNCTION()
	void InitializeCell(int maxStateNumber);

	UFUNCTION()
	void SetLocation(const FIntVector& newLocation);

	UFUNCTION()
	void CollapseToRandomState();

	UFUNCTION()
	bool UpdateStatesWithNeighbor(UGridCell* neighborCell, int directionIndex);
	
	UFUNCTION()
	void LogStates();

	UFUNCTION()
	void CollapseToEmptyState();

	UFUNCTION()
	float GetEntropy();

	UFUNCTION()
	void CollapseToState(int state, bool markCollapsed = true);

};
