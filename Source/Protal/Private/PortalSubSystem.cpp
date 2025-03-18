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
	/*
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APortal::StaticClass(), FoundActors);
	*/
	
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

		/*
		if (portal->WasRecentlyRendered())
		{
			// CaptureComponent->TextureTarget = portal->LinkedPortal->CaptureComponent->TextureTarget;
		}
		else
		{
			// CaptureComponent->TextureTarget = nullptr;
		}
		*/

		if (!portal->WasRecentlyRendered())
		{
			if (portal->LinkCamera != nullptr) CameraRelease(portal->LinkCamera);
			portal->LinkCamera = nullptr;
		}
		else
		{
			if (portal->LinkCamera == nullptr)
			{
				AActor* camera = CameraGet();

				if (camera == nullptr) return;
				
				portal->LinkCamera = camera;

				UActorComponent* cameraCapture = camera->GetComponentByClass(USceneCaptureComponent2D::StaticClass());

				if (cameraCapture == nullptr) continue;
		
				USceneCaptureComponent2D* capture = Cast<USceneCaptureComponent2D>(cameraCapture);

				if (capture == nullptr) continue;

				if (portal == nullptr) continue;
				
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
			
			RenderPortal(portal, portal->LinkCamera);
		}
	}

	for (AActor* camera : ActivePortalCameras)
	{
		UActorComponent* cameraCapture = camera->GetComponentByClass(USceneCaptureComponent2D::StaticClass());

		if (cameraCapture == nullptr) continue;
		
		USceneCaptureComponent2D* capture = Cast<USceneCaptureComponent2D>(cameraCapture);

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

AActor* UPortalSubSystem::CameraGet()
{
	if (PortalCameras.Num() == 0)
	{	
		UE_LOG(LogTemp, Warning, TEXT("New Camera Created!"));
		if (PortalCameraPrefab == nullptr) return nullptr;
		
		//Create New Camera
		AActor* camera = GetWorld()->SpawnActor(PortalCameraPrefab);

		//Create Render Target for the new camera
		UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
		RenderTarget->InitAutoFormat(512, 512);
		RenderTarget->UpdateResource();

		UActorComponent* cameraCapture = camera->GetComponentByClass(USceneCaptureComponent2D::StaticClass());

		if (cameraCapture == nullptr) return nullptr;
		
		USceneCaptureComponent2D* cam = Cast<USceneCaptureComponent2D>(cameraCapture);

		if (cam == nullptr) return nullptr;
		
		cam->TextureTarget = RenderTarget;
		cam->bEnableClipPlane = true;
		cam->bCaptureEveryFrame = false;
		
		PortalCameras.Add(camera);
		
		return camera;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Camera Reused!"));
		AActor* camera = PortalCameras.Pop();
		ActivePortalCameras.Add(camera);
		return camera;
	}
}

void UPortalSubSystem::CameraRelease(AActor* camera)
{
	if (camera == nullptr) return;

	if (ActivePortalCameras.Contains(camera))
	{
		ActivePortalCameras.Remove(camera);
		PortalCameras.Add(camera);
	}
}

void UPortalSubSystem::RenderPortal(APortal* portal, AActor* camera)
{
	if (portal == nullptr || camera == nullptr) return;
	
	UActorComponent* cameraCapture = camera->GetComponentByClass(USceneCaptureComponent2D::StaticClass());

	if (cameraCapture == nullptr) return;
	
	USceneCaptureComponent2D* capture = Cast<USceneCaptureComponent2D>(cameraCapture);

	if (capture == nullptr) return;
	
	capture->ClipPlaneNormal = portal->GetActorForwardVector();

	APawn* Player = GetLocalPlayer()->GetPlayerController(GetWorld())->GetPawn();
	
	camera->SetActorLocation(portal->RelativeLinkLocation(Player));
	
	camera->SetActorRotation(Player->GetControlRotation());
	
	capture->ClipPlaneBase = portal->GetActorLocation() + portal->GetActorForwardVector() * 50.0f; // Let's hope it works !
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
	UE_LOG(LogTemp, Warning, TEXT("Material slot count: %d"), MaterialSlotCount);

	if (MaterialSlotCount > 0)
	{
		portal->Mesh->SetMaterial(0, DynamicMaterial);
		UE_LOG(LogTemp, Warning, TEXT("Changed Material !"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No material slots available on portal mesh!"));
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