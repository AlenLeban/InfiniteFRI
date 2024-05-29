// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneratorRunnable.h"
#include "Engine/World.h"


FGeneratorRunnable::FGeneratorRunnable(UWorld* world, AFRIGenerator* generatorR, AWorldGenerator* worldGeneratorR, const FVector& loc)
{
	location = loc;
	worldRef = world;
	generatorRef = generatorR;
	worldGeneratorRef = worldGeneratorR;
	Thread = FRunnableThread::Create(this, TEXT("Give your thread a good name"));
}

FGeneratorRunnable::~FGeneratorRunnable()
{
	if (Thread)
	{
		// Kill() is a blocking call, it waits for the thread to finish.
		// Hopefully that doesn't take too long
		Thread->Kill();
		delete Thread;
	}
}

bool FGeneratorRunnable::Init()
{
	uint32 Seed = FPlatformTLS::GetCurrentThreadId() + 123; // Adjust the unique identifier as 
	generatorRef->randomGenerator.Initialize(Seed);
	return true;
}

uint32 FGeneratorRunnable::Run()
{
	/*while (bRunThread)
	{*/

	TArray<AFRIGenerator*> neighbors;

	{
		FScopeLock Lock(&MyCriticalSection);

		neighbors = worldGeneratorRef->GetNeighborGenerators(location);
	}

	//generatorRef->GenerateFRI(TArray<AFRIGenerator*>{});
	generatorRef->GenerateFRI(neighbors);
	
	AsyncTask(ENamedThreads::GameThread, [this]() {
		worldGeneratorRef->OnGeneratorFinished(generatorRef, this);
	});
	
	//}

	return 0;
}

//void FGeneratorRunnable::Exit()
//{
//}

void FGeneratorRunnable::Stop()
{
	bRunThread = false;
}
