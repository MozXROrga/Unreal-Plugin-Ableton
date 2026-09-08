// Copyright Ars Electronica Futurelab. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MoxAbletonComponent.generated.h"

class UMoxAbletonSubsystem;

/**
 * MoxAbletonComponent - Actor component for Ableton Live control.
 *
 * Attach this component to any actor to give it direct control over
 * Ableton Live clips, scenes, tracks, and song transport.
 *
 * Configure the Ableton Track/Scene/Clip IDs in the Details panel,
 * then call the control functions from Blueprint or C++.
 *
 * The component automatically registers/unregisters with the
 * MoxAbletonSubsystem on BeginPlay/EndPlay.
 *
 * Blueprint Category: MozXR|Ableton
 */
UCLASS(ClassGroup = (MozXR), meta = (BlueprintSpawnableComponent, DisplayName = "Mox Ableton Component"))
class MOXABLETON_API UMoxAbletonComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMoxAbletonComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// === Ableton Identifiers ===

	/** Ableton Track Number (1-based, as shown in Ableton Live). Set to 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MozXR|Ableton", meta = (ClampMin = "0"))
	int32 AbletonTrackID;

	/** Ableton Clip Slot Number (1-based, as shown in Ableton Live). Set to 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MozXR|Ableton", meta = (ClampMin = "0"))
	int32 AbletonClipSlot;

	/** Ableton Scene Number (1-based, as shown in Ableton Live). Set to 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MozXR|Ableton", meta = (ClampMin = "0"))
	int32 AbletonSceneID;

	// === Clip Control ===

	/** Fire the clip at the configured TrackID + ClipSlot */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Clip")
	void FireClip();

	/** Stop the clip at the configured TrackID + ClipSlot */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Clip")
	void StopClip();

	// === Scene Control ===

	/** Fire the scene at the configured SceneID */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Scene")
	void FireScene();

	// === Track Control ===

	/** Stop all clips on the configured TrackID */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Track")
	void StopTrack();

	/** Set volume of the configured TrackID (0.0 = silence, 1.0 = full) */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Track")
	void SetTrackVolume(float Volume);

	/** Mute/unmute the configured TrackID */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Track")
	void SetTrackMute(bool bMute);

	// === Song Control ===

	/** Start Ableton song playback */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Song")
	void StartSong();

	/** Stop Ableton song playback */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Song")
	void StopSong();

	// === Advanced ===

	/**
	 * Send any AbletonOSC command.
	 * @param EventPath OSC path WITHOUT /live/ prefix (e.g. "clip/set/name")
	 * @param Parameters Array of parameters as strings (auto-detected as int/float/string)
	 */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Advanced")
	void SendCustomEvent(const FString& EventPath, const TArray<FString>& Parameters);

	/** Check if this component is registered with the subsystem */
	UFUNCTION(BlueprintPure, Category = "MozXR|Ableton")
	bool IsRegistered() const { return bIsRegistered; }

	// === Listener (Subscribe using configured AbletonTrackID) ===

	/** Subscribe to volume changes on this component's track */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToVolume();

	/** Unsubscribe from volume changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromVolume();

	/** Subscribe to mute changes on this component's track */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToMute();

	/** Unsubscribe from mute changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromMute();

	/** Subscribe to panning changes on this component's track */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToPanning();

	/** Unsubscribe from panning changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromPanning();

	/** Subscribe to solo changes on this component's track */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToSolo();

	/** Unsubscribe from solo changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromSolo();

	/** Subscribe to which clip slot is currently playing on this track */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToPlayingSlot();

	/** Unsubscribe from playing slot changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromPlayingSlot();

private:
	UMoxAbletonSubsystem* GetSubsystem() const;

	bool bIsRegistered;
};
