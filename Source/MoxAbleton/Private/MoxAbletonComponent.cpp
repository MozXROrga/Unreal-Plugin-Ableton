// Copyright Ars Electronica Futurelab. All Rights Reserved.

#include "MoxAbletonComponent.h"
#include "MoxAbletonSubsystem.h"
#include "MoxAbletonTypes.h"

UMoxAbletonComponent::UMoxAbletonComponent()
	: AbletonTrackID(0)
	, AbletonClipSlot(0)
	, AbletonSceneID(0)
	, bIsRegistered(false)
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ============================================================================
// Lifecycle
// ============================================================================

void UMoxAbletonComponent::BeginPlay()
{
	Super::BeginPlay();

	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		bIsRegistered = Subsystem->RegisterComponent(this);
	}
}

void UMoxAbletonComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->UnregisterComponent(this);
	}
	bIsRegistered = false;

	Super::EndPlay(EndPlayReason);
}

// ============================================================================
// Helper
// ============================================================================

UMoxAbletonSubsystem* UMoxAbletonComponent::GetSubsystem() const
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GI)
	{
		return GI->GetSubsystem<UMoxAbletonSubsystem>();
	}
	return nullptr;
}

// ============================================================================
// Clip Control
// ============================================================================

void UMoxAbletonComponent::FireClip()
{
	if (AbletonTrackID < 1 || AbletonClipSlot < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot fire clip - Track(%d) or ClipSlot(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonTrackID, AbletonClipSlot);
		return;
	}

	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->FireClip(AbletonTrackID, AbletonClipSlot);
	}
}

void UMoxAbletonComponent::StopClip()
{
	if (AbletonTrackID < 1 || AbletonClipSlot < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot stop clip - Track(%d) or ClipSlot(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonTrackID, AbletonClipSlot);
		return;
	}

	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->StopClip(AbletonTrackID, AbletonClipSlot);
	}
}

// ============================================================================
// Scene Control
// ============================================================================

void UMoxAbletonComponent::FireScene()
{
	if (AbletonSceneID < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot fire scene - Scene(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonSceneID);
		return;
	}

	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->FireScene(AbletonSceneID);
	}
}

// ============================================================================
// Track Control
// ============================================================================

void UMoxAbletonComponent::StopTrack()
{
	if (AbletonTrackID < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot stop track - Track(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonTrackID);
		return;
	}

	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->StopTrack(AbletonTrackID);
	}
}

void UMoxAbletonComponent::SetTrackVolume(float Volume)
{
	if (AbletonTrackID < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot set volume - Track(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonTrackID);
		return;
	}

	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->SetTrackVolume(AbletonTrackID, Volume);
	}
}

void UMoxAbletonComponent::SetTrackMute(bool bMute)
{
	if (AbletonTrackID < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot set mute - Track(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonTrackID);
		return;
	}

	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->SetTrackMute(AbletonTrackID, bMute);
	}
}

// ============================================================================
// Song Control
// ============================================================================

void UMoxAbletonComponent::StartSong()
{
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->StartSong();
	}
}

void UMoxAbletonComponent::StopSong()
{
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->StopSong();
	}
}

// ============================================================================
// Custom Events
// ============================================================================

void UMoxAbletonComponent::SendCustomEvent(const FString& EventPath, const TArray<FString>& Parameters)
{
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->SendCustomEvent(EventPath, Parameters);
	}
}

// ============================================================================
// Listener Subscriptions (uses configured AbletonTrackID)
// ============================================================================

void UMoxAbletonComponent::SubscribeToVolume()
{
	if (AbletonTrackID < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot subscribe - Track(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonTrackID);
		return;
	}
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem) Subsystem->SubscribeToTrackVolume(AbletonTrackID);
}

void UMoxAbletonComponent::UnsubscribeFromVolume()
{
	if (AbletonTrackID < 1) return;
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem) Subsystem->UnsubscribeFromTrackVolume(AbletonTrackID);
}

void UMoxAbletonComponent::SubscribeToMute()
{
	if (AbletonTrackID < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot subscribe - Track(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonTrackID);
		return;
	}
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem) Subsystem->SubscribeToTrackMute(AbletonTrackID);
}

void UMoxAbletonComponent::UnsubscribeFromMute()
{
	if (AbletonTrackID < 1) return;
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem) Subsystem->UnsubscribeFromTrackMute(AbletonTrackID);
}

void UMoxAbletonComponent::SubscribeToPanning()
{
	if (AbletonTrackID < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot subscribe - Track(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonTrackID);
		return;
	}
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem) Subsystem->SubscribeToTrackPanning(AbletonTrackID);
}

void UMoxAbletonComponent::UnsubscribeFromPanning()
{
	if (AbletonTrackID < 1) return;
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem) Subsystem->UnsubscribeFromTrackPanning(AbletonTrackID);
}

void UMoxAbletonComponent::SubscribeToSolo()
{
	if (AbletonTrackID < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot subscribe - Track(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonTrackID);
		return;
	}
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem) Subsystem->SubscribeToTrackSolo(AbletonTrackID);
}

void UMoxAbletonComponent::UnsubscribeFromSolo()
{
	if (AbletonTrackID < 1) return;
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem) Subsystem->UnsubscribeFromTrackSolo(AbletonTrackID);
}

void UMoxAbletonComponent::SubscribeToPlayingSlot()
{
	if (AbletonTrackID < 1)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: %s: Cannot subscribe - Track(%d) not set (must be >= 1)"),
			*GetOwner()->GetName(), AbletonTrackID);
		return;
	}
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem) Subsystem->SubscribeToPlayingSlot(AbletonTrackID);
}

void UMoxAbletonComponent::UnsubscribeFromPlayingSlot()
{
	if (AbletonTrackID < 1) return;
	UMoxAbletonSubsystem* Subsystem = GetSubsystem();
	if (Subsystem) Subsystem->UnsubscribeFromPlayingSlot(AbletonTrackID);
}

