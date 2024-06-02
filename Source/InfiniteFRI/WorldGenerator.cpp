// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGenerator.h"
#include "GeneratorRunnable.h"
#include "Engine/StaticMeshActor.h"
#include "GridCell.h"

// Sets default values
AWorldGenerator::AWorldGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWorldGenerator::BeginPlay()
{
	Super::BeginPlay();
	if (generatorClass == nullptr) {
		return;
	}
	generatorOffset = stride * 200;
	locationToGeneratorMap = TMap<FVector, AFRIGenerator*>();
	if (automaticRadiusGeneration) {
		currentGeneratorLocations.Add(FVector(0, 0, 0));
		GenerateNextRooms(currentGeneratorLocations);
	}
}

void AWorldGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (int i = 0; i < generatorRunnables.Num(); i++) {
		if (generatorRunnables[i] != nullptr) {
			delete generatorRunnables[i];
		}
	}
}

// Called every frame
void AWorldGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWorldGenerator::OnGeneratorFinished(AFRIGenerator* genRef, FGeneratorRunnable* runnable)
{
	genRef->CopyCellsToWorldGenerator();
	genRef->MaterializeGrid();
	int nOfRemovedRunnables = generatorRunnables.Remove(runnable);
	delete runnable;
	if (nOfRemovedRunnables == 0) {
		return;
	}
	if (generatorRunnables.IsEmpty()) {
		if (generationRadius > currentRadius) {
			TArray<FVector> neighborUnitLocations = GetNeighborUnitLocationsOfGenerators(currentGeneratorLocations);
			currentGeneratorLocations = neighborUnitLocations;
			GenerateNextRooms(neighborUnitLocations);
		}
	}
	if(generatorRunnables.Num() < threadCount && !generatorQueue.IsEmpty() && !generatorLocationsQueue.IsEmpty()) {
		LaunchNextGenerator();
	}
}

void AWorldGenerator::GenerateNextRooms(TArray<FVector> roomUnitLocations)
{
	currentRadius++;
	int xOffsets[] = { 1, -1, 0, 0, 0, 0 };
	int yOffsets[] = { 0, 0, 1, -1, 0, 0 };
	int zOffsets[] = { 0, 0, 0, 0, 1, -1 };

	for (int i = 0; i < roomUnitLocations.Num(); i++) {
		FVector spawnLocation = GetActorLocation() + roomUnitLocations[i] * generatorOffset;
		AFRIGenerator* generatorRef = GetWorld()->SpawnActor<AFRIGenerator>(generatorClass, spawnLocation, FRotator());
		if (generatorRef == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("GeneratorRef failed to spawn"));
			return;
		}
		// create square generators
		generatorRef->gridWidth = generatorSize;
		generatorRef->gridLength = generatorSize;
		generatorRef->gridHeight = generatorSize;

		generatorRef->worldGeneratorRef = this;

		FVector generatorWorldUnitLocation = roomUnitLocations[i] * stride;
		FIntVector generatorWorldIntLocation = FIntVector(FMath::RoundToInt(generatorWorldUnitLocation.X), FMath::RoundToInt(generatorWorldUnitLocation.Y), FMath::RoundToInt(generatorWorldUnitLocation.Z));

		generatorRef->generatorWorldIntVector = generatorWorldIntLocation;
		generatorQueue.Enqueue(generatorRef);
		generatorLocationsQueue.Enqueue(roomUnitLocations[i]);
		// generatorRunnables.Add(new FGeneratorRunnable(GetWorld(), generatorRef, this, nextGeneratorLocationsCopy[i]));
		locationToGeneratorMap.Add(roomUnitLocations[i], generatorRef);

	}
	while (generatorRunnables.Num() < threadCount && !generatorQueue.IsEmpty() && !generatorLocationsQueue.IsEmpty()) {
		LaunchNextGenerator();
	}

}



TArray<AFRIGenerator*> AWorldGenerator::GetNeighborGenerators(const FVector& location)
{
	TArray<AFRIGenerator*> neighbors;
	int xOffsets[] = { 1, -1, 0, 0, 0, 0 };
	int yOffsets[] = { 0, 0, 1, -1, 0, 0 };
	int zOffsets[] = { 0, 0, 0, 0, 1, -1 };
	for (int j = 0; j < 6; j++) {
		FVector newLocation = location + FVector(xOffsets[j], yOffsets[j], zOffsets[j]);
		TArray<FVector> keys;
		locationToGeneratorMap.GetKeys(keys);
		for (int i = 0; i < locationToGeneratorMap.Num(); i++) {
			UE_LOG(LogTemp, Log, TEXT("%s"), *keys[i].ToString());
		}
		UE_LOG(LogTemp, Log, TEXT("KEY to compare to: %s"), *newLocation.ToString());
		if (locationToGeneratorMap.Contains(newLocation)) {
			UE_LOG(LogTemp, Log, TEXT("Contains"));
			neighbors.Add(locationToGeneratorMap[newLocation]);
		}
	}
	return neighbors;
}

void AWorldGenerator::LaunchNextGenerator()
{
	FVector location;
	AFRIGenerator* nextGenerator;
	generatorLocationsQueue.Dequeue(location);
	generatorQueue.Dequeue(nextGenerator);
	generatorRunnables.Add(new FGeneratorRunnable(GetWorld(), nextGenerator, this));
}

TArray<FVector> AWorldGenerator::GetNeighborUnitLocationsOfGenerators(TArray<FVector> locations)
{
	int xOffsets[] = { 1, -1, 0, 0, 0, 0 };
	int yOffsets[] = { 0, 0, 1, -1, 0, 0 };
	int zOffsets[] = { 0, 0, 0, 0, 1, -1 };
	TArray<FVector> neighborLocations;
	for (int i = 0; i < locations.Num(); i++) {
		for (int j = 0; j < 4; j++) {
			FVector newLocation = locations[i] + FVector(xOffsets[j], yOffsets[j], zOffsets[j]);
			if (!locationToGeneratorMap.Contains(newLocation) && !neighborLocations.Contains(newLocation)) {
				UE_LOG(LogTemp, Log, TEXT("New neighbor location in queue: %s"), *newLocation.ToString());
				neighborLocations.Add(newLocation);
			}
		}
	}
	return neighborLocations;
}

UGridCell* AWorldGenerator::GetCellAtWorldLocation(AFRIGenerator* askingGenerator, const FIntVector& location)
{
	if (!gridCellsMap.Contains(location)) {
		return nullptr;
	}
	return gridCellsMap[location];
}

void AWorldGenerator::SetCellAt(const FIntVector& location, UGridCell* newCell)
{
	if (newCell->isCollapsed && gridCellsMap.Contains(location) && gridCellsMap[location] != newCell) {
		if (gridCellsMap[location]->staticMeshActorRef != nullptr) {
			gridCellsMap[location]->staticMeshActorRef->Destroy();
		}
	}
	gridCellsMap.Add(location, newCell);
}

void AWorldGenerator::SpawnAndLaunchGeneratorAtUnitLocation(const FIntVector& location)
{
	FVector spawnLocation = FVector(location) * tileSize + GetActorLocation();
	AFRIGenerator* generatorRef = GetWorld()->SpawnActor<AFRIGenerator>(generatorClass, spawnLocation, FRotator());
	if (generatorRef == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("GeneratorRef failed to spawn"));
		return;
	}
	// create square generators
	generatorRef->gridWidth = generatorSize;
	generatorRef->gridLength = generatorSize;
	generatorRef->gridHeight = generatorSize;
	generatorRef->worldGeneratorRef = this;
	generatorRef->generatorWorldIntVector = location;
	generatorRunnables.Add(new FGeneratorRunnable(GetWorld(), generatorRef, this));
}

void AWorldGenerator::DestroyTilesAtUnitLocation(const FIntVector& location)
{
	for (int i = 0; i < generatorSize; i++) {
		for (int j = 0; j < generatorSize; j++) {
			for (int k = 0; k < generatorSize; k++) {
				FIntVector offset = FIntVector(i, j, k);
				FIntVector offsetLocation = location + offset;
				if (!gridCellsMap.Contains(offsetLocation)) {
					continue;
				}
				if (gridCellsMap[offsetLocation]->staticMeshActorRef != nullptr) {
					gridCellsMap[offsetLocation]->staticMeshActorRef->Destroy();
				}
				gridCellsMap.Remove(offsetLocation);
			}
		}
	}
}

