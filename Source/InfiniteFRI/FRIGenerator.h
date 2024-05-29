// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FRIGenerator.generated.h"

class UGridCell;
class AWorldGenerator;

DECLARE_DELEGATE(FMainThreadDelegate);


UCLASS()
class INFINITEFRI_API AFRIGenerator : public AActor
{

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* tileStatsDT;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* tileNameToSMDT;
	
public:	
	// Sets default values for this actor's properties
	AFRIGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int gridWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int gridLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int gridHeight;

	UPROPERTY()
	TArray<UGridCell*> gridCells;


	UPROPERTY()
	TArray<UGridCell*> cellHeap;
	
	UPROPERTY()
	TArray<float> stateEntropies;
	
	UPROPERTY()
	TArray<FName> tileStatsDTRowNames;

	UPROPERTY()
	FRandomStream randomGenerator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float gridCellSize = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int maxRetries = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isLogging = false;

	UPROPERTY()
	FIntVector generatorWorldIntVector;

	UPROPERTY()
	AWorldGenerator* worldGeneratorRef;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	

	UFUNCTION(BlueprintCallable)
	int GenerateFRI(TArray<AFRIGenerator*> neighborGrids, bool materialize = false);

	UFUNCTION()
	void CollapseEverythingToEmpty();

	UFUNCTION()
	void InitializeGrid();

	UFUNCTION()
	bool StartCollapsing();

	UFUNCTION()
	void ChooseRandomCell(FIntVector& location);

	UFUNCTION()
	void ChooseLowestEntropyCell(FIntVector& location);

	UFUNCTION()
	void CollapseCellAt(const FIntVector& location);

	UFUNCTION()
	UGridCell* GetGridCellAt(const FIntVector& location);

	UFUNCTION()
	int GetIndexFromLocation(const FIntVector& location);

	UFUNCTION()
	bool UpdateCellsFromLocation(const FIntVector& cellLocation);

	UFUNCTION()
	bool IsValidCellLocation(const FIntVector& location);

	UFUNCTION()
	void MaterializeCell(UGridCell* cell);

	UFUNCTION()
	void MaterializeGrid();

	UFUNCTION()
	void CollapseCellAtToEmptyState(const FIntVector& location);

	UFUNCTION()
	void CollapseCellAtToState(const FIntVector& location, int state, bool markCollapsed = true);

	UFUNCTION()
	void CopyConnectToGridTiles();

	UFUNCTION()
	void CopyNeighborGrids();

	UFUNCTION()
	void PrecomputeEntropyOfAllStates();

	UFUNCTION()
	void CopyCellsToWorldGenerator();

};
