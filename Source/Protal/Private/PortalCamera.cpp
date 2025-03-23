// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalCamera.h"
#include "Components/SceneCaptureComponent2D.h"

// Sets default values
APortalCamera::APortalCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Capture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));
	if (Capture == nullptr) return;
	SetRootComponent(Capture);
	
	Capture->bEnableClipPlane = true;
	Capture->bCaptureEveryFrame = false;
	Capture->bCaptureOnMovement = false;
	Capture->CaptureSource = ESceneCaptureSource::SCS_MAX;
	Capture->PostProcessSettings.AutoExposureMinBrightness = Capture->PostProcessSettings.AutoExposureMaxBrightness;
}