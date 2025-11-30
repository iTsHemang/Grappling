// Copyright Epic Games, Inc. All Rights Reserved.

#include "cpp_portCharacter.h"
#include "cpp_portCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// Acpp_portCharacter



Acpp_portCharacter::Acpp_portCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 90.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
	
	GraplOn = false;
	Gspeed = 500.0f;
	PreviousLocation = GetActorLocation();
}

void Acpp_portCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &Acpp_portCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &Acpp_portCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &Acpp_portCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &Acpp_portCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &Acpp_portCharacter::LookInput);

		EnhancedInputComponent->BindAction(Grapple, ETriggerEvent::Triggered, this, &Acpp_portCharacter::GrappleStart);
		EnhancedInputComponent->BindAction(GCut, ETriggerEvent::Triggered, this, &Acpp_portCharacter::GrappleCut);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void Acpp_portCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Grappling)
	{
		GrapplingRayCast();
	}

	if (Grappling)
	{
		RapplingIn(DeltaTime);
	}
}

void Acpp_portCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void Acpp_portCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void Acpp_portCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void Acpp_portCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void Acpp_portCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void Acpp_portCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void Acpp_portCharacter::GrapplingRayCast()
{
	FVector Start = GetActorLocation();
	FVector Forward = FirstPersonCameraComponent->GetForwardVector();
	Start = FVector(Start.X + (Forward.X * 100), Start.Y + (Forward.Y * 100), Start.Z + (Forward.Z * 100));
	FVector End = Start + (Forward * 5000);
	FHitResult Hit;

	if (GetWorld())
	{
		bool actorHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, FCollisionQueryParams(), FCollisionResponseParams());
		if (actorHit && Hit.GetActor())
		{
			float HitDistance = Hit.Distance;
			if (HitDistance <= 3000)
			{
				GraplOn = true;
				GPoint = Hit.ImpactPoint;
				PointNormal = Hit.ImpactNormal;
			}
			
			else 
			{
				GraplOn = false;
			}
		}
		
		else 
        {
        	GraplOn = false;
        }
	}
}

void Acpp_portCharacter::GrappleStart()
{
	if (GraplOn)
	{
		GraplOn = false;
		Grappling = true;
	}
}

void Acpp_portCharacter::GrappleCut()
{
	Grappling = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	GrappleCapleOff();
	
	if (Grappled)
	{
		Grappled = false;
		GetCharacterMovement()->GravityScale = 1.0f;
	}
}

void Acpp_portCharacter::RapplingIn(float DeltaTime)
{
	FVector CurrentLocation = GetActorLocation();
	FVector Direction = GPoint-CurrentLocation;
	float distance = Direction.Size();

	FVector MoveDelat = Direction.GetSafeNormal() * Gspeed * DeltaTime;
	GetCharacterMovement()->DisableMovement();

	float DeltaMove = FVector::DistSquared(CurrentLocation, PreviousLocation);
	FString DMS = FString::SanitizeFloat(DeltaMove);
	
	float UpNorm = FVector::DotProduct(PointNormal, FVector::UpVector);

	GrappleCapleOn(GPoint, DeltaMove);
	
	if (UpNorm > 0.7f)
	{
		//Floor
		if (DeltaMove < 1.0f)
		{
			Grappling = false;
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			GrappleCapleOff();
		}
	
		else
		{
			SetActorLocation(CurrentLocation + MoveDelat, true);
		}
	}

	else if (UpNorm < -0.7f)
	{
		//Ceiling

		if (DeltaMove < 1.0f)
		{
			Grappling = false;
			Grappled = true;
			Latch();
		}
	
		else
		{
			SetActorLocation(CurrentLocation + MoveDelat, true);
		}
	}

	else
	{
		//Wall
		if (DeltaMove < 1.0f)
		{
			Grappling = false;
			Grappled = true;
			Latch();
		}
	
		else
		{
			SetActorLocation(CurrentLocation + MoveDelat, true);
		}	
	}

	PreviousLocation = CurrentLocation;
}

void Acpp_portCharacter::Latch()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->GravityScale = 0.0f;
	GetCharacterMovement()->DisableMovement();
	GrappleCapleOff();
}
