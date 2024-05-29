// Fill out your copyright notice in the Description page of Project Settings.


#include "GridCell.h"
#include "FRIGenerator.h"
#include "TileStatsRow.h"



void UGridCell::InitializeCell(int maxStateNumber)
{
	states.Reserve(maxStateNumber + 1);
	for (int i = 0; i <= maxStateNumber; i++) {
		states.Add(i);
	}
}

void UGridCell::SetLocation(const FIntVector& newLocation)
{
	location = newLocation;
}

void UGridCell::CollapseToRandomState()
{

	if (states.Num() == 0) {
		states.Add(generatorRef->tileStatsDTRowNames.Num() - 1);
		areStatesDirty = true;
		isCollapsed = true;
		return;
	}

	int xOffsets[6] = { 1, -1, 0, 0, 0, 0 };
	int yOffsets[6] = { 0, 0, 1, -1, 0, 0 };
	int zOffsets[6] = { 0, 0, 0, 0, 1, -1 };

	FString directionKeys[] = {"x+", "x-", "y+", "y-", "z+", "z-"};

	TArray<float> boundaries;
	boundaries.Reserve(states.Num());
	float cumulativeProb = 0;
	float emptyStateProb = 0.001;
	for (int s = 0; s < states.Num(); s++) {
		int state = states[s];
		if (state == generatorRef->tileStatsDTRowNames.Num() - 1) {
			// is empty state
			cumulativeProb += emptyStateProb;
			boundaries.Add(cumulativeProb);
			continue;
		}
		FName stateName = generatorRef->tileStatsDTRowNames[state];
		float stateFakeEntropy = 0;
		float stateProb = 1;
		for (int i = 0; i < 6; i++) {
			FIntVector neighborLocation = location + FIntVector(xOffsets[i], yOffsets[i], zOffsets[i]);
			if (!generatorRef->IsValidCellLocation(neighborLocation)) {
				continue;
			}
			int neighborCellIndex = generatorRef->GetIndexFromLocation(neighborLocation);
			UGridCell* neighborCell = generatorRef->gridCells[neighborCellIndex];
			float withNeighborEntropy = 0;
			float withNeighborProb = 0;
			for (int j = 0; j < neighborCell->states.Num(); j++) {
				int neighborState = neighborCell->states[j];
				FName neighborStateName = generatorRef->tileStatsDTRowNames[neighborState];
				FTileStatsRow* row = generatorRef->tileStatsDT->FindRow<FTileStatsRow>(neighborStateName, "context");
				int oppositeDirection = i % 2 == 0 ? i + 1 : i - 1;
				FString directionKey = directionKeys[oppositeDirection];
				TArray<FString> possibleStatesKeys;
				row->mappings[directionKey].dirToCountMap.GetKeys(possibleStatesKeys);
				if (!possibleStatesKeys.Contains(generatorRef->tileStatsDTRowNames[state])) {
					continue;
				}
				float stateProbWithNeighbor = row->mappings[directionKey].dirToCountMap[generatorRef->tileStatsDTRowNames[state].ToString()];
				withNeighborEntropy += -stateProbWithNeighbor * FMath::Log2(stateProbWithNeighbor);
				withNeighborProb += stateProbWithNeighbor;
			}
			stateProb *= withNeighborProb;
			stateFakeEntropy += withNeighborEntropy;
		}
		cumulativeProb += 1 / stateFakeEntropy;
		//cumulativeProb += stateProb;
		boundaries.Add(cumulativeProb);
	}

	float randomFloat = generatorRef->randomGenerator.GetFraction();
	int stateIndex = 0;
	for (int s = 0; s < boundaries.Num(); s++) {
		if (generatorRef->isLogging)
			UE_LOG(LogTemp, Log, TEXT("Boundary: %f"), boundaries[s]);
		boundaries[s] /= cumulativeProb;
		if (randomFloat < boundaries[s]) {
			stateIndex = s;
			break;
		}
	}

	
	int collapsedState = states[stateIndex];
	states.Empty();
	states.Add(collapsedState);
	areStatesDirty = true;
	isCollapsed = true;
	
}

bool UGridCell::UpdateStatesWithNeighbor(UGridCell* neighborCell, int directionIndex)
{
	if (generatorRef->isLogging)
		UE_LOG(LogTemp, Warning, TEXT("Updating cell at %s"), *location.ToString());
	FString mappingKeys[6] = { "x+", "x-", "y+", "y-", "z+", "z-" };
	if (generatorRef->isLogging)
		UE_LOG(LogTemp, Warning, TEXT("Direction: %s"), *mappingKeys[directionIndex]);
	if (neighborCell->states.Num() == 0) {
		if (generatorRef->isLogging)
			UE_LOG(LogTemp, Error, TEXT("Cell has no state, returning false"));
		return false;
	}
	int stateIndex = 0;
	bool didStatesChange = false;
	if (generatorRef->isLogging)
		UE_LOG(LogTemp, Warning, TEXT("Printing neighbor's states: "));
	neighborCell->LogStates();
	while (stateIndex < states.Num()) {
		int state = states[stateIndex];
		//UE_LOG(LogTemp, Warning, TEXT("Checking state %d"), state);
		FName rowName = generatorRef->tileStatsDTRowNames[state];
		FTileStatsRow* row = generatorRef->tileStatsDT->FindRow<FTileStatsRow>(rowName, "context");
		FString key = mappingKeys[directionIndex];
		if (!row->mappings.Contains(key)) {
			if (generatorRef->isLogging)
				UE_LOG(LogTemp, Warning, TEXT("State has no mapping for set direction"));
			didStatesChange = true;
			states.RemoveAt(stateIndex);
			areStatesDirty = true;
			continue;
		}
		FTileStatsMappingRow mappingValues = row->mappings[key];
		TSet<FString> keys;
		mappingValues.dirToCountMap.GetKeys(keys);
		bool wasMatchFound = false;
		
		for (int i = 0; i < neighborCell->states.Num(); i++) {
			int neighborState = neighborCell->states[i];
			FName neighborRowName = generatorRef->tileStatsDTRowNames[neighborState];
			
			if (keys.Contains(neighborRowName.ToString())) {
				//UE_LOG(LogTemp, Warning, TEXT("Neighbor has a matching state: %d"), neighborState);
				wasMatchFound = true;
				break;
			}
		}

		if (!wasMatchFound) {
			if (generatorRef->isLogging)
				UE_LOG(LogTemp, Warning, TEXT("Neighbor has no matching state, removing."));
			didStatesChange = true;
			states.RemoveAt(stateIndex);
			areStatesDirty = true;
			continue;
		}
		stateIndex++;
	}
	if (generatorRef->isLogging)
		UE_LOG(LogTemp, Warning, TEXT("State changed: %b"), didStatesChange);
	return didStatesChange;
}

void UGridCell::LogStates()
{
	FString statesString = "";
	for (int i = 0; i < states.Num(); i++) {
		statesString += FString::FromInt(states[i]) + ", ";
	}
	if (generatorRef->isLogging) {
		UE_LOG(LogTemp, Log, TEXT("Cell: %20s: states: %s"), *location.ToString(), *statesString);
		UE_LOG(LogTemp, Log, TEXT("Entropy: %f"), GetEntropy());
	}
}

void UGridCell::CollapseToEmptyState()
{
	states.Empty();
	states.Add(generatorRef->tileStatsDTRowNames.Num()-1); // last one is empty
	isCollapsed = true;
}

float UGridCell::GetEntropy()
{
	if (states.Num() == 0) {
		entropy = 0;
		areStatesDirty = false;
		return entropy;
	}
	float emptyStateEntropy = 100;
	if (areStatesDirty) {
		if (states.Num() == 1) {
			areStatesDirty = false;
			entropy = 0;
		}
		else if(states.Num() > 1) {
			//entropy = states.Num();
			entropy = 0;
			for (int i = 0; i < states.Num(); i++) {
				if (states[i] == generatorRef->tileStatsDTRowNames.Num() - 1) {
					// is empty state
					entropy += emptyStateEntropy;
					continue;
				}
				entropy += generatorRef->stateEntropies[states[i]];
			}
			areStatesDirty = false;
		}
		else {
			entropy = 99999;
		}
	}
	return entropy;
}

void UGridCell::CollapseToState(int state, bool markCollapsed)
{
	int collapsedState = state;
	states.Empty();
	states.Add(collapsedState);
	areStatesDirty = true;
	isCollapsed = markCollapsed;

}
