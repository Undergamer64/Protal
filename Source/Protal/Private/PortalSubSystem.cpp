// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalSubSystem.h"

#include "EngineUtils.h"
#include "Portal.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

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
	FVector2d NewResolution = GetGameResolution();
	FVector2d NewViewportSize = GetGameViewportSize();

	FMatrix Matrix = GetPlayerProjectionMatrix();
	
	if (Resolution != NewResolution || ViewportSize != NewViewportSize)
	{
		Resolution = NewResolution;
		ViewportSize = NewViewportSize;
		//FOV = NewFOV;

		for (APortalCamera* camera : ActivePortalCameras)
		{
			if (camera == nullptr || camera->Capture || camera->Capture->TextureTarget) continue;

			UTextureRenderTarget2D* RenderTarget = camera->Capture->TextureTarget;
			
			RenderTarget->InitAutoFormat(Resolution.X, Resolution.Y);
			RenderTarget->UpdateResource();
			camera->Capture->CustomProjectionMatrix = Matrix;

			camera->Capture->FOVAngle = FOV;
		}
	}
	
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
			if (portal->LinkCamera != nullptr)
			{
				CameraRelease(portal->LinkCamera);
				
				portal->LinkCamera = nullptr;

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

				UTexture* BlankTexture;
				PortalMaterialPrefab->GetTextureParameterValue(FName("RenderTarget"), BlankTexture);
				
				DynamicMat->SetTextureParameterValue(FName("RenderTarget"), BlankTexture);
				UE_LOG(LogTemp, Warning, TEXT("Updated RenderTexture Parameter!"));
			}
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
				camera->Capture->HiddenActors.Add(portal->LinkedPortal);
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
	if (PortalCameras.Num() == 0)
	{	
		UE_LOG(LogTemp, Warning, TEXT("New Camera Created!"));
		if (PortalCameraPrefab == nullptr) return nullptr;
		
		//Create New Camera
		APortalCamera* camera = Cast<APortalCamera>(GetWorld()->SpawnActor(PortalCameraPrefab));

		//Create Render Target for the new camera
		UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
		RenderTarget->InitAutoFormat(Resolution.X, Resolution.Y);
		RenderTarget->UpdateResource();
		
		USceneCaptureComponent2D* cam = camera->Capture;

		if (cam == nullptr) return nullptr;
		
		cam->TextureTarget = RenderTarget;
		
		ActivePortalCameras.Add(camera);
		
		camera->Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
		
		return camera;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Camera Reused!"));
		APortalCamera* camera = PortalCameras.Pop();
		camera->Capture->HiddenActors.Empty();
		ActivePortalCameras.Add(camera);
		return camera;
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

	if (Player == nullptr)
	{
		APawn* pawn = GetLocalPlayer()->GetPlayerController(GetWorld())->GetPawn();

		if (pawn == nullptr) return;

		Player = Cast<AProtalCharacter>(pawn);
		
		if (Player == nullptr) return;
	}
	
	portal->LinkCamera->SetActorLocation(portal->RelativeLinkLocation(Player->GetFirstPersonCameraComponent()->GetComponentLocation()));
	
	portal->LinkCamera->SetActorRotation(portal->RelativeLinkRotation(Player->GetControlRotation().Quaternion()));

	portal->LinkedPortal->SetupClipPlane();
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

FVector2D UPortalSubSystem::GetGameViewportSize()
{
	FVector2D Result = FVector2D( 1, 1 );

	if ( GEngine && GEngine->GameViewport )
	{
		GEngine->GameViewport->GetViewportSize( /*out*/Result );
	}

	return Result;
}

FVector2D UPortalSubSystem::GetGameResolution()
{
	FVector2D Result = FVector2D( 1, 1 );

	Result.X = GSystemResolution.ResX;
	Result.Y = GSystemResolution.ResY;

	return Result;
}

APlayerCameraManager* UPortalSubSystem::GetPlayerCameraManager() const
{
	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
		{
			return PlayerController->PlayerCameraManager;
		}
	}
	return nullptr;
}

FMatrix UPortalSubSystem::GetPlayerProjectionMatrix() const
{
	APlayerCameraManager* CameraManager = GetPlayerCameraManager();
	if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("No PlayerCameraManager found!"));
		return FMatrix::Identity;
	}

	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		FSceneViewProjectionData ProjectionData;
		if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport,ProjectionData))
		{
			return ProjectionData.ProjectionMatrix;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Failed to retrieve Projection Matrix!"));
	return FMatrix::Identity;
}