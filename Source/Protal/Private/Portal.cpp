// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"

// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> portalMesh(TEXT("/Game/TestPortal.TestPortal"));
	ConstructorHelpers::FObjectFinder<UMaterialInterface> portalMaterial(TEXT("/Game/Protal.Protal"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Mesh->SetMobility(EComponentMobility::Static);
	Mesh->CastShadow = false;
	Mesh->bReceivesDecals = false;
	if (portalMesh.Object && portalMaterial.Object) {
		Mesh->SetStaticMesh(portalMesh.Object);
		Mesh->SetMaterial(0, portalMaterial.Object);
	}
	SetRootComponent(Mesh);

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetMobility(EComponentMobility::Static);
	Trigger->SetBoxExtent(FVector(0, 100, 100));
	Trigger->SetRelativeLocation(FVector(-60, 0, 0));
	//Trigger->OnComponentEndOverlap.AddDynamic(this, &APortal::TeleportAttempt);
	Trigger->SetupAttachment(Mesh);
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();
	
}

void APortal::TeleportAttempt(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (LinkedPortal == nullptr || OtherActor == nullptr) return;

	OtherActor->SetActorLocation(LinkedPortal->GetActorLocation());
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector APortal::RelativeLinkLocation(AActor* actor) const
{
	if (LinkedPortal == nullptr || actor == nullptr) return FVector::ZeroVector;
	return LinkedPortal->GetActorTransform().InverseTransformPosition(GetActorTransform().TransformPosition(actor->GetActorLocation()));
}
