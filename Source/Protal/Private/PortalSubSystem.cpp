// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalSubSystem.h"

#include "EngineUtils.h"
#include "Portal.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Systems/MovieSceneQuaternionBlenderSystem.h"

UPortalSubSystem::UPortalSubSystem()
{
	ConstructorHelpers::FObjectFinder<UClass> Camera(TEXT("/Game/Cam.Cam_C"));
	ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("/Game/Protal.Protal"));

	if (Camera.Succeeded())
	{
		PortalCameraPrefab = Camera.Object;
	}

	if (Material.Succeeded())
	{
		PortalMaterialPrefab = Material.Object;
	}
}

void UPortalSubSystem::Init()
{
	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UPortalSubSystem::OnWorldInitializedActors);
}

void UPortalSubSystem::OnWorldInitializedActors(const FActorsInitializedParams& _)
{
	for (TActorIterator<APortal> It(GetWorld()); It; ++It)
	{
		CreateNewPortal(*It);
	}
}

// FTickableGameObject Begin
void UPortalSubSystem::Tick(float DeltaTime)
{
	for (APortal* portal : Portals)
	{
		if (portal == nullptr)
		{
			Portals.Remove(portal);
			continue;
		}
		if (portal->LinkedPortal == nullptr)
		{
			if (portal->LinkCamera != nullptr)
			{
				CameraRelease(portal->LinkCamera);
				portal->LinkCamera = nullptr;
			}
			continue;
		}

		if (!portal->WasRecentlyRendered())
		{
			if (portal->LinkCamera != nullptr) CameraRelease(portal->LinkCamera);
			portal->LinkCamera = nullptr;
		}
		else
		{
			if (portal->LinkCamera == nullptr)
			{
				APortalCamera* camera = CameraGet();

				if (camera == nullptr) return;
				
				portal->LinkCamera = camera;
		
				USceneCaptureComponent2D* capture = camera->Capture;

				if (capture == nullptr) continue;
				
				UMaterialInterface* BaseMaterial = portal->Mesh->GetMaterial(0);

				UMaterialInstanceDynamic* DynamicMat = Cast<UMaterialInstanceDynamic>(BaseMaterial);

				if (DynamicMat == nullptr)
				{
					DynamicMat = UMaterialInstanceDynamic::Create(PortalMaterialPrefab, portal);
					if (!DynamicMat)
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to create Dynamic Material Instance!"));
						continue;
					}

					portal->Mesh->SetMaterial(0, DynamicMat);
				}

				if (capture && capture->TextureTarget)
				{
					DynamicMat->SetTextureParameterValue(FName("RenderTarget"), capture->TextureTarget);
					UE_LOG(LogTemp, Warning, TEXT("Updated RenderTexture Parameter!"));
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Capture or TextureTarget is NULL!"));
					continue;
				}
			}
			
			RenderPortal(portal);
		}
	}

	for (APortalCamera* camera : ActivePortalCameras)
	{
		USceneCaptureComponent2D* capture = camera->Capture;

		if (capture == nullptr) continue;
		
		capture->CaptureScene();
	}
}

ETickableTickType UPortalSubSystem::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

TStatId UPortalSubSystem::GetStatId() const 
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FMyTickableThing, STATGROUP_Tickables);
}

bool UPortalSubSystem::IsTickableWhenPaused() const
{
	return true;
}

bool UPortalSubSystem::IsTickableInEditor() const
{
	return false;
}
// FTickableGameObject End

APortalCamera* UPortalSubSystem::CameraGet()
{
	if (PortalCameras.Num() == 0 && ActivePortalCameras.Num() == 0)
	{	
		UE_LOG(LogTemp, Warning, TEXT("New Camera Created!"));
		if (PortalCameraPrefab == nullptr) return nullptr;
		
		//Create New Camera
		APortalCamera* camera = Cast<APortalCamera>(GetWorld()->SpawnActor(PortalCameraPrefab));

		//Create Render Target for the new camera
		UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
		RenderTarget->InitAutoFormat(512, 512);
		RenderTarget->UpdateResource();
		
		USceneCaptureComponent2D* cam = camera->Capture;

		if (cam == nullptr) return nullptr;
		
		cam->TextureTarget = RenderTarget;
		cam->bEnableClipPlane = true;
		cam->bCaptureEveryFrame = false;
		
		ActivePortalCameras.Add(camera);
		
		return camera;
	}
	else if (PortalCameras.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Camera Reused!"));
		APortalCamera* camera = PortalCameras.Pop();
		ActivePortalCameras.Add(camera);
		return camera;
	}
	else
	{
		return nullptr;
	}
}

void UPortalSubSystem::CameraRelease(APortalCamera* camera)
{
	if (camera == nullptr) return;

	if (ActivePortalCameras.Contains(camera))
	{
		ActivePortalCameras.Remove(camera);
		PortalCameras.Add(camera);
	}
}

void UPortalSubSystem::RenderPortal(APortal* portal)
{
	if (portal == nullptr || portal->LinkCamera == nullptr || portal->LinkedPortal == nullptr) return;
	
	APawn* Player = GetLocalPlayer()->GetPlayerController(GetWorld())->GetPawn();
	
	portal->LinkCamera->SetActorLocation(portal->RelativeLinkLocation(Player->GetActorLocation()));
	
	portal->LinkCamera->SetActorRotation(portal->RelativeLinkRotation(Player->GetControlRotation().Quaternion()));

	USceneCaptureComponent2D* capture = portal->LinkCamera->Capture;

	if (capture == nullptr) return;

	portal->LinkedPortal->SetupClipPlane(capture);
}

void UPortalSubSystem::CreateNewPortal(APortal* portal)
{
	if (portal == nullptr) return;
	
	Portals.Add(portal);

	if (PortalMaterialPrefab == nullptr) return;
	
	//Create Material Here
	UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(PortalMaterialPrefab, portal);
	
	if (DynamicMaterial == nullptr) return;

	if (portal->Mesh == nullptr) return;
	
	int32 MaterialSlotCount = portal->Mesh->GetNumMaterials();

	if (MaterialSlotCount > 0)
	{
		portal->Mesh->SetMaterial(0, DynamicMaterial);
	}
}

void UPortalSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("UMyLocalPlayerSubsystem Initialized!"));
	Init();
}

void UPortalSubSystem::Deinitialize()
{
	Super::Deinitialize();
}