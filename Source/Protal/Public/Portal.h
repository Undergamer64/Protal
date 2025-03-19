// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PortalCamera.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

UCLASS()
class PROTAL_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortal();
	
	void TeleportAttempt(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY(Category = Portal, EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(Category = Portal, EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> Trigger;

	UPROPERTY(Category = Portal, EditAnywhere)
	APortal* LinkedPortal = nullptr;

	UPROPERTY()
	APortalCamera* LinkCamera = nullptr;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector RelativeLinkLocation(FVector Pos) const;

	FQuat RelativeLinkRotation(FQuat Rot) const;

	void SetupClipPlane(USceneCaptureComponent2D* CaptureComponent) const;
};
