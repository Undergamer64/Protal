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
	
	void Init();

	void OnWorldInitializedActors(const FActorsInitializedParams& _);

	virtual void UPortalSubSystem::Tick(float DeltaTime) override;

	virtual ETickableTickType GetTickableTickType() const override;

	virtual TStatId GetStatId() const override;

	virtual bool IsTickableWhenPaused() const override;

	virtual bool IsTickableInEditor() const override;

	void CreateNewPortal(APortal* portal);

private :
	UPROPERTY()
	TArray<AActor*> ActivePortalCameras;
	
	UPROPERTY()
	TArray<AActor*> PortalCameras;

	AActor* CameraGet();
	void CameraRelease(AActor*);

	UPROPERTY()
	TArray<APortal*> LastActivePortals = TArray<APortal*>();

	void RenderPortal(APortal* portal, AActor* camera);
};
