// Fill out your copyright notice in the Description page of Project Settings.


#include "FRIGenerator.h"
#include "GridCell.h"
#include "Engine/DataTable.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "NameToStaticMeshRow.h"
#include "WorldGenerator.h"
#include "TileStatsRow.h"
// Sets default values
AFRIGenerator::AFRIGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
}

// Called when the game starts or when spawned
void AFRIGenerator::BeginPlay()
{
	Super::BeginPlay();
	// Seed the random number generator using the thread ID and a unique identifier
	uint32 Seed = FPlatformTLS::GetCurrentThreadId() + 123; // Adjust the unique identifier as needed
	randomGenerator.Initialize(Seed);
}

// Called every frame
void AFRIGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int AFRIGenerator::GenerateFRI(bool materialize)
{
	if (tileNameToSMDT == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("tileNameToSMDT not set"));
		return -1;
	}
	if (isLogging)
		UE_LOG(LogTemp, Log, TEXT("GenerateFRI"));
	bool wasCollapseSuccessful = false;
	while (!wasCollapseSuccessful && retries < maxRetries) {
		UpdateProgressVisual();
		retries++;
		UE_LOG(LogTemp, Warning, TEXT("Collapsing, retries: %d"), retries);
		InitializeGrid();
		bool wasSuccessfulCopy = CopyConnectToGridTiles();
		if (!wasSuccessfulCopy) {
			UE_LOG(LogTemp, Error, TEXT("Unsuccessful copy of neighbor cells"));
			retries = maxRetries;
			break;
		}
		
		if (generatorWorldIntVector == FIntVector(5, 0, 0)) {
			break;
		}

		wasCollapseSuccessful = StartCollapsing();
	}
	UE_LOG(LogTemp, Warning, TEXT("Retry count: %d"), retries);
	if (retries >= maxRetries) {
		UE_LOG(LogTemp, Error, TEXT("Max retries reached!"));
		CollapseEverythingToEmpty();
	}
	if (materialize) {
		MaterializeGrid();
		if (worldGeneratorRef != nullptr) {
			CopyCellsToWorldGenerator();
		}
	}
	OnGeneratorFinished();
	return retries;
}

void AFRIGenerator::CollapseEverythingToEmpty() 
{
	for (int i = 0; i < gridCells.Num(); i++) {
		gridCells[i]->CollapseToEmptyState();
	}
}


void AFRIGenerator::InitializeGrid()
{
	DrawDebugBox(GetWorld(), GetActorLocation() - FVector(gridCellSize / 2) + FVector(gridWidth, gridLength, gridHeight) / 2 * gridCellSize, FVector(gridWidth, gridLength, gridHeight) * gridCellSize / 2, FColor::Cyan, false, 5, (uint8)0U, 5);
	UE_LOG(LogTemp, Log, TEXT("InitializeGrid"));
	gridCells.Empty();
	cellHeap.Empty();
	gridCells.Reserve(gridWidth * gridHeight * gridLength);
	cellHeap.Reserve(gridWidth * gridHeight * gridLength);

	if (isLogging)
		UE_LOG(LogTemp, Warning, TEXT("InitializeGrid"));
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridLength; j++) {
			for (int k = 0; k < gridHeight; k++) {
				UGridCell* newGridCell = NewObject<UGridCell>();
				newGridCell->SetLocation(FIntVector(i, j, k));
				newGridCell->generatorRef = this;
				newGridCell->InitializeCell(tileStatsDT->GetRowNames().Num()-1);
				gridCells.Add(newGridCell);
				cellHeap.Add(newGridCell);
			}
		}
	}

	tileStatsDTRowNames = tileStatsDT->GetRowNames();
	PrecomputeEntropyOfAllStates();
	cellHeap.Heapify(UGridCell::gridCellPredicate);

	UE_LOG(LogTemp, Log, TEXT("Finished initializing grid"));
}

bool AFRIGenerator::StartCollapsing()
{
	FIntVector cellLocation;
	//ChooseRandomCell(cellLocation);
	//CollapseCellAt(cellLocation);
	//UpdateCellsFromLocation(cellLocation);
	int loopCount = 0;
	int maxLoopCount = gridWidth * gridLength * gridHeight * 2;
	while (cellHeap.Num() > 0 && loopCount < maxLoopCount) {
		ChooseLowestEntropyCell(cellLocation);
		CollapseCellAt(cellLocation);
		bool updateSuccess = UpdateCellsFromLocation(cellLocation);
		if (!updateSuccess) {
			return false;
		}
		loopCount++;
	}
	if (loopCount >= maxLoopCount) {
		if (isLogging)
			UE_LOG(LogTemp, Error, TEXT("MaxLoopCountReached"));
		return false;
	}
	return true;
}

void AFRIGenerator::ChooseRandomCell(FIntVector& location)
{
	UGridCell* chosenCell = gridCells[FMath::RandRange(0, gridCells.Num() - 1)];
	location.X = chosenCell->location.X;
	location.Y = chosenCell->location.Y;
	location.Z = chosenCell->location.Z;
}

void AFRIGenerator::ChooseLowestEntropyCell(FIntVector& location)
{
	UGridCell* bestCell = cellHeap.HeapTop();
	location.X = bestCell->location.X;
	location.Y = bestCell->location.Y;
	location.Z = bestCell->location.Z;
	if (isLogging)
		UE_LOG(LogTemp, Log, TEXT("bestCell: %s"), *bestCell->location.ToString());
}

void AFRIGenerator::CollapseCellAt(const FIntVector& location)
{
	if (isLogging)
		UE_LOG(LogTemp, Warning, TEXT("Collapsed cell at %s"), *location.ToString());
	//UE_LOG(LogTemp, Log, TEXT("L1"));
	UGridCell* cellToCollapse = GetGridCellAt(location);
	//UE_LOG(LogTemp, Log, TEXT("cellToCollapse: stateCount: %d"), cellToCollapse->states.Num());
	cellToCollapse->CollapseToRandomState();
	//UE_LOG(LogTemp, Log, TEXT("L3"));
	//MaterializeCell(cellToCollapse);
	//UE_LOG(LogTemp, Log, TEXT("L4"));
	int heapIndex = 0;
	cellHeap.Find(cellToCollapse, heapIndex);
	if (!cellHeap.IsValidIndex(heapIndex)) {
		UE_LOG(LogTemp, Error, TEXT("CollapseCellAt: Invalid heap index at CollapseCellAt: %d"), heapIndex);
		return;
	}
	//UE_LOG(LogTemp, Log, TEXT("L5"));
	cellHeap.HeapRemoveAt(heapIndex, UGridCell::gridCellPredicate);
	//UE_LOG(LogTemp, Log, TEXT("L6"));
}

UGridCell* AFRIGenerator::GetGridCellAt(const FIntVector& location)
{
	return gridCells[location.X * (gridLength * gridHeight) + location.Y * (gridHeight) + location.Z];
}

int AFRIGenerator::GetIndexFromLocation(const FIntVector& location)
{
	return location.X * (gridLength * gridHeight) + location.Y * gridHeight + location.Z;
}

bool AFRIGenerator::UpdateCellsFromLocation(const FIntVector& cellLocation)
{
	int neighborIndexOffsets[6] = {
		gridLength * gridHeight,
		-gridLength * gridHeight,
		gridHeight,
		-gridHeight,
		1,
		-1
	};

	int xOffsets[] = { 1, -1, 0, 0, 0, 0 };
	int yOffsets[] = { 0, 0, 1, -1, 0, 0 };
	int zOffsets[] = { 0, 0, 0, 0, 1, -1 };

	TQueue<UGridCell*> cellsToCheckQueue;
	int startingCellIndex = GetIndexFromLocation(cellLocation);
	UGridCell* startingCell = gridCells[startingCellIndex];
	if (startingCell->states.Num() == 0) {
		if (isLogging)
			UE_LOG(LogTemp, Error, TEXT("UpdateCellsFromLocation: starting cell has no states, skipping."));
		return false;
	}
	cellsToCheckQueue.Enqueue(startingCell);
	int checkIndex = FMath::Rand();
	startingCell->checkIndex = checkIndex;
	int maxCellsCheck = 100;
	int cellsChecked = 0;
	while (!cellsToCheckQueue.IsEmpty() && cellsChecked < maxCellsCheck) {
		cellsChecked++;
		UGridCell* cellToCheck;
		cellsToCheckQueue.Dequeue(cellToCheck);
		int cellToCheckIndex = GetIndexFromLocation(cellToCheck->location);
		//UE_LOG(LogTemp, Log, TEXT("CellToCheck: %s"), *cellToCheck->location.ToString());

		if (cellToCheck->states.IsEmpty()) {
			return false;
		}

		TArray<UGridCell*> generatedNeighbors;
		generatedNeighbors.Reserve(6);

		bool didCheckedCellChange = cellToCheck->isCollapsed;
		for (int i = 0; i < 6; i++) {
			FIntVector neighborLocation = cellToCheck->location + FIntVector(xOffsets[i], yOffsets[i], zOffsets[i]);
			if (!IsValidCellLocation(neighborLocation)) {
				continue;
			}
			int neighborCellIndex = cellToCheckIndex + neighborIndexOffsets[i];
			UGridCell* neighborCell = gridCells[neighborCellIndex];
			didCheckedCellChange |= cellToCheck->UpdateStatesWithNeighbor(neighborCell, i);
			generatedNeighbors.Add(neighborCell);
		}
		if (cellToCheck->states.Num() == 0) {
			// conflict state
			return false;
		}
		if (didCheckedCellChange) {
			for (int i = 0; i < generatedNeighbors.Num(); i++) {
				if (generatedNeighbors[i]->checkIndex != checkIndex && !generatedNeighbors[i]->isCollapsed) {
					if (generatedNeighbors[i]->states.IsEmpty()) {
						return false;
					}
					generatedNeighbors[i]->checkIndex = checkIndex;
					cellsToCheckQueue.Enqueue(generatedNeighbors[i]);
				}
			}
		}

		cellToCheck->LogStates();
	}
	return true;
}

bool AFRIGenerator::IsValidCellLocation(const FIntVector& location)
{
	return location.X >= 0 && location.X < gridWidth && location.Y >= 0 && location.Y < gridLength && location.Z >= 0 && location.Z < gridHeight;
}

void AFRIGenerator::MaterializeCell(UGridCell* cell)
{
	/*if (cell->location.X == 0 || cell->location.Y == 0 || cell->location.Z == 0 ||
		cell->location.X == gridWidth - 1 || cell->location.Y == gridLength - 1 || cell->location.Z == gridHeight - 1) {
		return;
	}*/
	if (cell->states.Num() == 0 || !cell->isCollapsed) {
		if (isLogging)
			UE_LOG(LogTemp, Log, TEXT("MaterializeCell: Cell at %s has no states"), *cell->location.ToString());
		return;
	}
	FName rowName = tileStatsDTRowNames[cell->states[0]];
	FTileStatsRow* tileRow = tileStatsDT->FindRow<FTileStatsRow>(rowName, "context");
	FRotator spawnRotation = FRotator(0, tileRow->rotation * 90, 0);
	FVector spawnLocation = GetActorLocation() + FVector(cell->location.X, cell->location.Y, cell->location.Z) * gridCellSize;
	AStaticMeshActor* MyNewActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), spawnLocation, spawnRotation);
	//MyNewActor->SetActorLabel(*rowName.ToString().Append(cell->location.ToString()));
	MyNewActor->SetMobility(EComponentMobility::Stationary);
	UStaticMeshComponent* MeshComponent = MyNewActor->GetStaticMeshComponent();
	FName cellStateName = FName((tileStatsDTRowNames[cell->states[0]].ToString()).LeftChop(2));
	FNameToStaticMeshRow* sm = tileNameToSMDT->FindRow<FNameToStaticMeshRow>(cellStateName, "context");
	if (MeshComponent && sm != nullptr && sm->staticMesh.Num() > 0)
	{
		cell->staticMeshActorRef = MyNewActor;
		int randomIndex = FMath::RandRange(0, sm->staticMesh.Num() - 1);
		MeshComponent->SetStaticMesh(sm->staticMesh[randomIndex]);
	}
}

void AFRIGenerator::MaterializeGrid()
{
	for (int i = 0; i < gridCells.Num(); i++) {
		MaterializeCell(gridCells[i]);
	}
}

void AFRIGenerator::CollapseCellAtToEmptyState(const FIntVector& location)
{
	if (isLogging)
		UE_LOG(LogTemp, Warning, TEXT("Collapsed cell at %s"), *location.ToString());
	UGridCell* cellToCollapse = GetGridCellAt(location);
	cellToCollapse->CollapseToEmptyState();
	//MaterializeCell(cellToCollapse);
	int heapIndex = 0;
	cellHeap.Find(cellToCollapse, heapIndex);
	if (!cellHeap.IsValidIndex(heapIndex)) {
		UE_LOG(LogTemp, Error, TEXT("CollapseCellAtToEmptyState: Invalid heap index at CollapseCellAt: %d"), heapIndex);
		return;
	}
	cellHeap.HeapRemoveAt(heapIndex, UGridCell::gridCellPredicate);
}

void AFRIGenerator::CollapseCellAtToState(const FIntVector& location, int state, bool markCollapsed)
{
	if (isLogging)
		UE_LOG(LogTemp, Warning, TEXT("Collapsed cell at %s"), *location.ToString());
	UGridCell* cellToCollapse = GetGridCellAt(location);
	cellToCollapse->CollapseToState(state, markCollapsed);
	int heapIndex = 0;
	cellHeap.Find(cellToCollapse, heapIndex);
	if (!cellHeap.IsValidIndex(heapIndex)) {
		UE_LOG(LogTemp, Error, TEXT("CollapseCellAtToState: Invalid heap index at CollapseCellAt: %d"), heapIndex);
		return;
	}
	cellHeap.HeapRemoveAt(heapIndex, UGridCell::gridCellPredicate);
}

bool AFRIGenerator::CopyConnectToGridTiles()
{
	int copyIndent = 0;
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridLength; j++) {
			for (int k = 0; k < gridHeight; k++) {
				// if it's a tile at the edge of the generator grid
				if (i == copyIndent || j == copyIndent || k == copyIndent || i == gridWidth - (copyIndent + 1) || j == gridLength - (copyIndent + 1) || k == gridHeight - (copyIndent + 1)) {
					FIntVector cellRelativeIntLocation = FIntVector(i, j, k);
					//UE_LOG(LogTemp, Log, TEXT("Generator world int vector: %s"), *generatorWorldIntVector.ToString());
					FIntVector tileIntWorldLocation = generatorWorldIntVector + cellRelativeIntLocation;
					//UE_LOG(LogTemp, Log, TEXT("GeneratorWorldIntVector: %s"), *generatorWorldIntVector.ToString());
					//UE_LOG(LogTemp, Log, TEXT("Tile location: %s"), *tileIntWorldLocation.ToString());
					UGridCell* copyingCell = worldGeneratorRef->GetCellAtWorldLocation(this, tileIntWorldLocation);
					if (copyingCell == nullptr) {
						// might not be a collapsed cell or generator not available at all
						//UE_LOG(LogTemp, Warning, TEXT("Copying cell == nullptr at %s"), *tileIntWorldLocation.ToString());
						continue;
					}
					if (!copyingCell->states.IsValidIndex(0)) {
						//UE_LOG(LogTemp, Error, TEXT("Invalid cell state index when copying neighbor: %d"), 0);
						CollapseCellAtToState(cellRelativeIntLocation, tileStatsDTRowNames.Num()-1, true);
						UpdateCellsFromLocation(cellRelativeIntLocation);
						continue;
					}
					CollapseCellAtToState(cellRelativeIntLocation, copyingCell->states[0], true);
					bool successfulUpdate = UpdateCellsFromLocation(cellRelativeIntLocation);
					if (!successfulUpdate) {
						UE_LOG(LogTemp, Error, TEXT("Unsuccessful update"));
						return false;
					}
					//UE_LOG(LogTemp, Log, TEXT("Successful copy"));
				}
			}
		}
	}
	return true;

	//for (int i = 0; i < gridWidth; i++) {
	//	for (int j = 0; j < gridLength; j++) {
	//		for (int k = 0; k < gridHeight; k++) {
	//			// if it's a tile at the edge of the generator grid
	//			if (i == 0 || j == 0 || k == 0 || i == gridWidth - 1 || j == gridLength - 1 || k == gridHeight - 1) {
	//				FIntVector cellRelativeIntLocation = FIntVector(i, j, k);

	//				bool successfulUpdate = UpdateCellsFromLocation(cellRelativeIntLocation);
	//				if (!successfulUpdate) {
	//					UE_LOG(LogTemp, Error, TEXT("Unsuccessful update"));
	//				}
	//				//UE_LOG(LogTemp, Log, TEXT("Successful copy"));
	//			}
	//		}
	//	}
	//}

	//MaterializeGrid();

	UE_LOG(LogTemp, Log, TEXT("Finished copying neighbor"));

}

void AFRIGenerator::CopyNeighborGrids()
{
	CopyConnectToGridTiles();
}

void AFRIGenerator::PrecomputeEntropyOfAllStates()
{
	stateEntropies.Reserve(tileStatsDTRowNames.Num());
	for (int i = 0; i < tileStatsDTRowNames.Num(); i++) {
		FName stateName = tileStatsDTRowNames[i];
		FTileStatsRow* row = tileStatsDT->FindRow<FTileStatsRow>(stateName, "context");
		TArray<FString> mappingKeys;
		row->mappings.GetKeys(mappingKeys);
		float stateEntropy = 0;
		for (int j = 0; j < mappingKeys.Num(); j++) {
			TArray<FString> otherStateKeys;
			TArray<float> newProbabilities;
			TMap<FString, float> mappingRow = row->mappings[mappingKeys[j]].dirToCountMap;
			mappingRow.GetKeys(otherStateKeys);
			if (otherStateKeys.Num() == 0) {
				continue;
			}
			newProbabilities.Reserve(otherStateKeys.Num());
			float cumulativeProbability = 0;
			for (int k = 0; k < otherStateKeys.Num(); k++) {
				cumulativeProbability += mappingRow[otherStateKeys[k]];
			}
			for (int k = 0; k < otherStateKeys.Num(); k++) {
				newProbabilities.Add(mappingRow[otherStateKeys[k]] / cumulativeProbability);
			}
			for (int k = 0; k < newProbabilities.Num(); k++) {
				stateEntropy += -newProbabilities[k] * FMath::Log2(newProbabilities[k]);
			}
		}
		stateEntropies.Add(stateEntropy);
	}
}

void AFRIGenerator::CopyCellsToWorldGenerator()
{
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridLength; j++) {
			for (int k = 0; k < gridHeight; k++) {
				int cellIndex = GetIndexFromLocation(FIntVector(i, j, k));
				FIntVector mapCellLocation = generatorWorldIntVector + FIntVector(i, j, k);
				if (!gridCells[cellIndex]->isCollapsed) {
					continue;
				}
				worldGeneratorRef->SetCellAt(mapCellLocation, gridCells[cellIndex]);
			}
		}
	}
}
