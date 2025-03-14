// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalSubSystem.h"

#include "EngineUtils.h"
#include "Portal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

void UPortalSubSystem::Init(USceneCaptureComponent2D* capture)
{
	CaptureComponent = capture;

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
			return;
		}
		if (portal->LinkedPortal == nullptr)
		{
			if (portal->LinkCamera != nullptr)
			{
				CameraRelease(portal->LinkCamera);
				portal->LinkCamera = nullptr;
			}
			return;
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
			CameraRelease(portal->LinkCamera);
			portal->LinkCamera = nullptr;
		}
		else
		{
			if (portal->LinkCamera == nullptr)
			{
				AActor* camera = CameraGet();

				if (camera == nullptr) return;
				
				portal->LinkCamera = camera;

				USceneCaptureComponent2D* capture = Cast<USceneCaptureComponent2D>(camera->GetComponentByClass(USceneCaptureComponent2D::StaticClass()));

				if (capture == nullptr) continue;
				
				UMaterialInstanceDynamic* DynamicMat = Cast<UMaterialInstanceDynamic>(portal->Mesh->GetMaterial(0));

				if (DynamicMat == nullptr) continue;

				DynamicMat->SetTextureParameterValue(FName("RenderTexture"), capture->TextureTarget);

				RenderPortal(portal, camera);
			}
		}
	}

	for (AActor* camera : ActivePortalCameras)
	{
		USceneCaptureComponent2D* capture = Cast<USceneCaptureComponent2D>(camera->GetComponentByClass(USceneCaptureComponent2D::StaticClass()));

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
		if (PortalCameraPrefab == nullptr) return nullptr;
		
		//Create New Camera
		AActor* camera = GetWorld()->SpawnActor(PortalCameraPrefab);
		PortalCameras.Add(camera);

		//Create Render Target for the new camera
		UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
		RenderTarget->InitAutoFormat(512, 512);
		RenderTarget->UpdateResource();
		
		USceneCaptureComponent2D* cam = Cast<USceneCaptureComponent2D>(camera->GetComponentByClass(USceneCaptureComponent2D::StaticClass()));

		if (cam == nullptr) return nullptr;
		
		cam->TextureTarget = RenderTarget;
		cam->bEnableClipPlane = true;
		cam->bCaptureEveryFrame = false;
		
		return camera;
	}
	else
	{
		AActor* camera = PortalCameras.Pop();
		ActivePortalCameras.Add(camera);
		return camera;
	}
}

void UPortalSubSystem::CameraRelease(AActor* camera)
{
	ActivePortalCameras.Remove(camera);
	PortalCameras.Add(camera);
}

void UPortalSubSystem::RenderPortal(APortal* portal, AActor* camera)
{
	USceneCaptureComponent2D* capture = Cast<USceneCaptureComponent2D>(camera->GetComponentByClass(USceneCaptureComponent2D::StaticClass()));

	if (capture == nullptr) return;
	
	capture->ClipPlaneNormal = portal->GetActorForwardVector();
	
	camera->SetActorLocation(portal->RelativeLinkLocation(camera));
}

void UPortalSubSystem::CreateNewPortal(APortal* portal)
{
	if (portal == nullptr) return;
	
	Portals.Add(portal);

	if (PortalMaterialPrefab == nullptr) return;
	
	//Create Material Here
	UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(PortalMaterialPrefab, portal);

	portal->Mesh->SetMaterial(0, DynamicMaterial);
}