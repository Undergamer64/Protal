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
class UPortalSubSystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

	UPortalSubSystem();
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UPROPERTY()
	TSubclassOf<AActor> PortalCameraPrefab;
	
	UPROPERTY()
	UMaterial* PortalMaterialPrefab;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	TArray<APortal*> Portals;
	
	void Init();

	void OnWorldInitializedActors(const FActorsInitializedParams& _);

	virtual void Tick(float DeltaTime) override;

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
