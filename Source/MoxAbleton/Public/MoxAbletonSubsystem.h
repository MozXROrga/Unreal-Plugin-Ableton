// Copyright Ars Electronica Futurelab. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MoxAbletonTypes.h"
#include "MoxAbletonListener.h"
#include "MoxAbletonSubsystem.generated.h"

class UMoxAbletonComponent;
class UMoxAbletonListener;
class FSocket;

/**
 * MoxAbletonSubsystem manages the OSC connection to Ableton Live via AbletonOSC.
 *
 * Provides full control over Ableton Live:
 * - Song transport (start/stop playback)
 * - Scene triggering (fire/stop scenes)
 * - Clip control (fire/stop clips by track + slot)
 * - Track control (play/stop, volume, mute)
 * - Custom OSC events (any AbletonOSC command)
 * - Emergency stop (panic button)
 *
 * IMPORTANT: All Track/Clip/Scene IDs use 1-based numbering
 * matching Ableton Live's visual display.
 * Internally, the subsystem converts to 0-based OSC indices.
 *
 * AbletonOSC Reference: https://github.com/ideoforms/AbletonOSC
 * Send Port: 11000 (default)
 * Receive Port: 11001 (default)
 *
 * Configuration via MozConfig.ini [moxableton] section.
 *
 * Lifecycle:
 * 1. Initialize(): Loads INI, creates socket
 * 2. StartAbletonSystem(): Enables command sending
 * 3. Runtime: Components call Fire/Stop/Set functions
 * 4. StopAbletonSystem(): Disables sending
 * 5. Deinitialize(): Emergency stop, cleanup
 */
UCLASS()
class MOXABLETON_API UMoxAbletonSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMoxAbletonSubsystem();

	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// === System Control ===

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton")
	void StartAbletonSystem();

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton")
	void StopAbletonSystem();

	UFUNCTION(BlueprintPure, Category = "MozXR|Ableton")
	bool IsAbletonSystemRunning() const { return bSystemEnabled; }

	UFUNCTION(BlueprintPure, Category = "MozXR|Ableton")
	EMoxAbletonConnectionState GetConnectionState() const { return ConnectionState; }

	// === Configuration ===

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton")
	void SetAbletonAddress(const FString& IP, int32 Port);

	// Reload configuration from MozConfig.ini
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton")
	void LoadConfigurationFromINI(bool bAutoStart = false);

	// === Song Control (Public) ===

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Song")
	void StartSong();

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Song")
	void StopSong();

	// === Emergency Controls (Public) ===

	/** Emergency: Stop ALL clips in entire Ableton project */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton")
	void StopAllClips();

	// === Scene Control ===

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Scene")
	void FireScene(int32 SceneID);

	// === Clip Control ===

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Clip")
	void FireClip(int32 TrackID, int32 ClipSlot);

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Clip")
	void StopClip(int32 TrackID, int32 ClipSlot);

	// === Track Control ===

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Track")
	void StopTrack(int32 TrackID);

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Track")
	void SetTrackVolume(int32 TrackID, float Volume);

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Track")
	void SetTrackMute(int32 TrackID, bool bMute);

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Track")
	void SetTrackPanning(int32 TrackID, float Panning);

	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Track")
	void SetTrackSolo(int32 TrackID, bool bSolo);

	// === Device Control ===

	/** Set a device parameter value */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Device")
	void SetDeviceParameter(int32 TrackID, int32 DeviceID, int32 ParameterID, float Value);

	// === Custom Events (Advanced) ===

	/**
	 * Send any AbletonOSC command.
	 * @param EventPath OSC path WITHOUT /live/ prefix (e.g. "clip/set/name")
	 * @param Parameters Array of parameters as strings (auto-detected as int/float/string)
	 */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Advanced")
	void SendCustomEvent(const FString& EventPath, const TArray<FString>& Parameters);

	// === Listener (Receive from AbletonOSC) ===

	/** Start the OSC listener to receive events from AbletonOSC (default port 11001) */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void StartListening();

	/** Stop the OSC listener */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void StopListening();

	/** Check if the listener is running */
	UFUNCTION(BlueprintPure, Category = "MozXR|Ableton|Listener")
	bool IsListening() const;

	// --- Song Subscriptions ---

	/** Subscribe to beat events from Ableton */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToBeat();

	/** Unsubscribe from beat events */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromBeat();

	/** Subscribe to tempo changes from Ableton */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToTempo();

	/** Unsubscribe from tempo changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromTempo();

	/** Subscribe to play state changes from Ableton */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToIsPlaying();

	/** Unsubscribe from play state changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromIsPlaying();

	/** Subscribe to song position changes from Ableton */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToCurrentSongTime();

	/** Unsubscribe from song position changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromCurrentSongTime();

	// --- Track Subscriptions (1-based TrackID) ---

	/** Subscribe to track volume changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToTrackVolume(int32 TrackID);

	/** Unsubscribe from track volume changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromTrackVolume(int32 TrackID);

	/** Subscribe to track mute changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToTrackMute(int32 TrackID);

	/** Unsubscribe from track mute changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromTrackMute(int32 TrackID);

	/** Subscribe to track panning changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToTrackPanning(int32 TrackID);

	/** Unsubscribe from track panning changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromTrackPanning(int32 TrackID);

	/** Subscribe to track solo changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToTrackSolo(int32 TrackID);

	/** Unsubscribe from track solo changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromTrackSolo(int32 TrackID);

	/** Subscribe to which clip is currently playing on a track */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToPlayingSlot(int32 TrackID);

	/** Unsubscribe from playing slot changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromPlayingSlot(int32 TrackID);

	/** Subscribe to which clip was just triggered on a track */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToFiredSlot(int32 TrackID);

	/** Unsubscribe from fired slot changes */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromFiredSlot(int32 TrackID);

	/** Subscribe to track output meter level (audio-reactive) */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToTrackOutputMeter(int32 TrackID);

	/** Unsubscribe from track output meter */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromTrackOutputMeter(int32 TrackID);

	// --- Scene Subscriptions (1-based SceneID) ---

	/** Subscribe to scene triggered state */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToSceneTriggered(int32 SceneID);

	/** Unsubscribe from scene triggered state */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromSceneTriggered(int32 SceneID);

	// --- Device Subscriptions (1-based IDs) ---

	/** Subscribe to a specific device parameter value */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToDeviceParameter(int32 TrackID, int32 DeviceID, int32 ParameterID);

	/** Unsubscribe from device parameter */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromDeviceParameter(int32 TrackID, int32 DeviceID, int32 ParameterID);

	// --- Generic ---

	/** Subscribe to any AbletonOSC property. Indices are 0-based OSC indices. */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void SubscribeToCustomProperty(const FString& Category, const FString& Property, const TArray<int32>& Indices);

	/** Unsubscribe from a custom property */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeFromCustomProperty(const FString& Category, const FString& Property, const TArray<int32>& Indices);

	/** Unsubscribe from all active listeners */
	UFUNCTION(BlueprintCallable, Category = "MozXR|Ableton|Listener")
	void UnsubscribeAll();

	// === Listener Delegates (bind these in Blueprint) ===

	/** Fires every beat */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonBeat OnBeatReceived;

	/** Fires when tempo changes */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonTempoChanged OnTempoChanged;

	/** Fires when play/stop state changes */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonPlayStateChanged OnPlayStateChanged;

	/** Fires when song position changes */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonSongTimeChanged OnSongTimeChanged;

	/** Fires when track volume changes (TrackID is 1-based) */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonTrackFloatChanged OnTrackVolumeChanged;

	/** Fires when track panning changes */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonTrackFloatChanged OnTrackPanningChanged;

	/** Fires when track output meter level changes */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonTrackFloatChanged OnTrackOutputMeterChanged;

	/** Fires when track mute changes */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonTrackBoolChanged OnTrackMuteChanged;

	/** Fires when track solo changes */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonTrackBoolChanged OnTrackSoloChanged;

	/** Fires when the playing clip slot changes on a track */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonTrackIntChanged OnPlayingSlotChanged;

	/** Fires when a clip slot is triggered on a track */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonTrackIntChanged OnFiredSlotChanged;

	/** Fires when a scene is triggered */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonSceneTriggered OnSceneTriggered;

	/** Fires when a device parameter changes */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonDeviceParameterChanged OnDeviceParameterChanged;

	/** Generic: fires for any OSC message received from AbletonOSC */
	UPROPERTY(BlueprintAssignable, Category = "MozXR|Ableton|Listener")
	FOnAbletonOSCReceived OnOSCReceived;

	// === Component Registration ===

	bool RegisterComponent(UMoxAbletonComponent* Component);
	void UnregisterComponent(UMoxAbletonComponent* Component);

	/** Get direct access to the listener object (for Component use) */
	UMoxAbletonListener* GetListener() const { return Listener; }

private:
	// Friend class for component access
	friend class UMoxAbletonComponent;

	// Load INI settings
	void LoadINISettings();

	// Send OSC packet to Ableton
	void SendToAbleton(const FString& Address, const TArray<uint8>& PacketData, int32 PacketSize);

	// === State ===

	FSocket* AbletonSocket;
	bool bSystemEnabled;
	EMoxAbletonConnectionState ConnectionState;

	// === Configuration ===

	FString AbletonIP;
	int32 AbletonPort;
	int32 ReceivePort;

	// === Component Registry ===

	TMap<FString, TWeakObjectPtr<UMoxAbletonComponent>> RegisteredComponents;

	// === Listener ===

	UPROPERTY()
	TObjectPtr<UMoxAbletonListener> Listener;

	// === Logging Configuration ===

	bool bLogCommands;      // Log clip/scene/track/song commands
	bool bLogOSCMessages;   // Log raw OSC message details (verbose)
	bool bLogCustomEvents;  // Log custom event invocations
};
