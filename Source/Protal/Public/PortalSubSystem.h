// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PortalSubSystem.generated.h"

/**
 * 
 */
UCLASS()
class PROTAL_API UPortalSubSystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> PortalCameraPrefab;
	
	UPROPERTY(EditAnywhere)
	UMaterial* PortalMaterialPrefab;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	TArray<APortal*> Portals;

	UPROPERTY()
	USceneCaptureComponent2D* CaptureComponent;
	
	void Init(USceneCaptureComponent2D* capture);

	void OnWorldInitializedActors(const FActorsInitializedParams& _);

	void UPortalSubSystem::Tick(float DeltaTime) override;

	ETickableTickType GetTickableTickType() const override;

	TStatId GetStatId() const override;

	bool IsTickableWhenPaused() const override;

	bool IsTickableInEditor() const override;

	void CreateNewPortal(APortal* portal);

private :
	TArray<AActor*> ActivePortalCameras;
	TArray<AActor*> PortalCameras;

	AActor* CameraGet();
	void CameraRelease(AActor*);

	TArray<APortal*> LastActivePortals = TArray<APortal*>();

	void RenderPortal(APortal* portal, AActor* camera);
};
