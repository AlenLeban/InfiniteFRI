// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGenerator.generated.h"

class AFRIGenerator;
class FGeneratorRunnable;

UCLASS()
class INFINITEFRI_API AWorldGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AFRIGenerator> generatorClass;

public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	TArray<FRunnable*> generatorRunnables;

	TQueue<AFRIGenerator*> generatorQueue;

	TQueue<FVector> generatorLocationsQueue;

	UPROPERTY()
	TSet<FVector> generatorLocations;

	UPROPERTY()
	TMap<FVector, AFRIGenerator*> locationToGeneratorMap;
	
	UPROPERTY()
	TArray<FVector> currentGeneratorLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int generatorOffset = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int generatorSize = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int stride = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int generationRadius = 3;
	
	UPROPERTY()
	int currentRadius = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int threadCount = 2;

	UPROPERTY()
	TMap<FIntVector, UGridCell*> gridCellsMap;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OnGeneratorFinished(AFRIGenerator* genRef, FGeneratorRunnable* runnable);

	UFUNCTION()
	void GenerateNextRooms(TArray<FVector> roomUnitLocations);

	UFUNCTION()
	TArray<AFRIGenerator*> GetNeighborGenerators(const FVector& location);

	UFUNCTION()
	void LaunchNextGenerator();

	UFUNCTION()
	TArray<FVector> GetNeighborUnitLocationsOfGenerators(TArray<FVector> locations);

	UFUNCTION()
	UGridCell* GetCellAtWorldLocation(AFRIGenerator* askingGenerator, const FIntVector& location);

};
