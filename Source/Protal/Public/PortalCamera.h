// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalCamera.generated.h"

UCLASS()
class PROTAL_API APortalCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortalCamera();

	TObjectPtr<USceneCaptureComponent2D> Capture;
};
