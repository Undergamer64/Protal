// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"

#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> portalMesh(TEXT("/Game/Plane.Plane"));
	ConstructorHelpers::FObjectFinder<UMaterialInterface> portalMaterial(TEXT("/Game/TestPortal.TestPortal"));
	
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
	Trigger->OnComponentEndOverlap.AddDynamic(this, &APortal::TeleportAttempt);
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

	if (OtherActor->IsA(ACharacter::StaticClass()))
	{
		ACharacter* Character = Cast<ACharacter>(OtherActor);
		
		UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
		UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Character->GetRootComponent());
		if (MovementComponent != nullptr && Root != nullptr)
		{
			FVector Velocity = MovementComponent->Velocity;

			Root->SetPhysicsLinearVelocity(Velocity);
		}

		FQuat Rotation = RelativeLinkRotation(Character->GetControlRotation().Quaternion());

		FRotator Rotator = Rotation.Rotator();

		Rotator.Roll = 0;
		
		Character->GetController()->SetControlRotation(Rotator);
	}
	else
	{
		FQuat Rotation = RelativeLinkRotation(OtherActor->GetActorQuat());

		FRotator Rotator = FRotator(Rotation.Rotator().Yaw, Rotation.Rotator().Pitch, 0);
	
		OtherActor->SetActorRotation(Rotator);
	}
	
	OtherActor->SetActorLocation(RelativeLinkLocation(OtherActor->GetActorLocation()), false, nullptr, ETeleportType::TeleportPhysics);

	OtherActor->SetActorScale3D(OtherActor->GetActorScale() + (LinkedPortal->GetActorScale() - GetActorScale()));
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector APortal::RelativeLinkLocation(FVector Pos) const
{
	if (LinkedPortal == nullptr) return FVector::ZeroVector;

	FVector RelativeLocation = GetActorTransform().InverseTransformPosition(Pos);
	
	FVector FlippedRelativeLocation = FVector(-RelativeLocation.X, -RelativeLocation.Y, RelativeLocation.Z);
	
	FVector EndLocation = LinkedPortal->GetTransform().TransformPosition(FlippedRelativeLocation);
	
	return EndLocation;
}

FQuat APortal::RelativeLinkRotation(FQuat Rot) const
{
	if (!LinkedPortal)
		return FQuat::Identity;

	FQuat localIn = GetActorQuat().Inverse() * Rot;
	
	FQuat EndRotation = LinkedPortal->GetActorQuat() * (FQuat(0,0,1,0) * localIn);
	
	return EndRotation;
}

void APortal::SetupClipPlane() const
{
	if (LinkCamera == nullptr || LinkedPortal == nullptr) return;
	
	if (!LinkCamera->Capture)
	{
		UE_LOG(LogTemp, Warning, TEXT("No SceneCaptureComponent found!"));
		return;
	}
	
	LinkCamera->Capture->ClipPlaneNormal = LinkedPortal->GetActorForwardVector();
	LinkCamera->Capture->ClipPlaneBase = LinkedPortal->GetActorLocation() - LinkedPortal->GetActorForwardVector() * 200;
}
