// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HAL/Runnable.h"
#include "FRIGenerator.h"
#include "WorldGenerator.h"


/**
 * 
 */
class FGeneratorRunnable : public FRunnable
{

public:

	FGeneratorRunnable(UWorld* world, AFRIGenerator* generatorR, AWorldGenerator* worldGeneratorR, const FVector& loc);

	virtual ~FGeneratorRunnable() override;

	//override Init,Run and Stop.
	virtual bool Init() override;
	virtual uint32 Run() override;
	//virtual void Exit() override;
	virtual void Stop() override;

	FCriticalSection MyCriticalSection;

private:
	// Thread handle. Control the thread using this, with operators like Kill and Suspend
	UPROPERTY()
	FRunnableThread* Thread;

	// Used to know when the thread should exit, changed in Stop(), read in Run()
	bool bRunThread;

	UPROPERTY()
	AFRIGenerator* generatorRef;

	UPROPERTY()
	UWorld* worldRef;

	UPROPERTY()
	AWorldGenerator* worldGeneratorRef;

	UPROPERTY()
	FVector location;

	UPROPERTY()
	FMainThreadDelegate mainThreadDelegate;
};
