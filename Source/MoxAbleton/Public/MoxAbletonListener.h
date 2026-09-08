// Copyright Ars Electronica Futurelab. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MoxAbletonTypes.h"
#include "MoxAbletonListener.generated.h"

class UOSCServer;
class UMoxAbletonSubsystem;
struct FOSCMessage;
struct FOSCAddress;

/**
 * MoxAbletonListener - Receives OSC messages from AbletonOSC.
 *
 * Manages a UOSCServer on port 11001 (AbletonOSC response port) and routes
 * incoming messages to Blueprint-assignable delegates.
 *
 * The artist explicitly subscribes to events they need:
 *   SubscribeToBeat()           -> OnBeatReceived
 *   SubscribeToTempo()          -> OnTempoChanged
 *   SubscribeToTrackVolume(ID)  -> OnTrackVolumeChanged
 *   ...
 *
 * Subscribe sends /live/xxx/start_listen/xxx to Ableton via the subsystem.
 * Unsubscribe sends /live/xxx/stop_listen/xxx.
 *
 * All TrackIDs are 1-based (matching Ableton UI). Internally converted to
 * 0-based OSC indices for communication.
 */
UCLASS()
class MOXABLETON_API UMoxAbletonListener : public UObject
{
	GENERATED_BODY()

public:
	UMoxAbletonListener();

	// === Lifecycle ===

	/** Initialize the listener with a reference to the owning subsystem */
	void Setup(UMoxAbletonSubsystem* InOwnerSubsystem);

	/** Start the OSC server to receive messages from AbletonOSC */
	void StartListening(int32 Port = 11001);

	/** Stop the OSC server and unsubscribe all active listeners */
	void StopListening();

	/** Check if the OSC server is running */
	bool IsListening() const { return bIsListening; }

	// === Song Subscriptions (Global) ===

	/** Subscribe to beat events. Fires OnBeatReceived every beat. */
	void SubscribeToBeat();
	void UnsubscribeFromBeat();

	/** Subscribe to tempo changes. Fires OnTempoChanged when BPM changes. */
	void SubscribeToTempo();
	void UnsubscribeFromTempo();

	/** Subscribe to play state. Fires OnPlayStateChanged when playing/stopped. */
	void SubscribeToIsPlaying();
	void UnsubscribeFromIsPlaying();

	/** Subscribe to song position. Fires OnSongTimeChanged with beat position. */
	void SubscribeToCurrentSongTime();
	void UnsubscribeFromCurrentSongTime();

	// === Track Subscriptions (per Track, 1-based TrackID) ===

	/** Subscribe to track volume changes */
	void SubscribeToTrackVolume(int32 TrackID);
	void UnsubscribeFromTrackVolume(int32 TrackID);

	/** Subscribe to track mute changes */
	void SubscribeToTrackMute(int32 TrackID);
	void UnsubscribeFromTrackMute(int32 TrackID);

	/** Subscribe to track panning changes */
	void SubscribeToTrackPanning(int32 TrackID);
	void UnsubscribeFromTrackPanning(int32 TrackID);

	/** Subscribe to track solo changes */
	void SubscribeToTrackSolo(int32 TrackID);
	void UnsubscribeFromTrackSolo(int32 TrackID);

	/** Subscribe to playing slot index (which clip is playing on a track) */
	void SubscribeToPlayingSlot(int32 TrackID);
	void UnsubscribeFromPlayingSlot(int32 TrackID);

	/** Subscribe to fired slot index (which clip was triggered on a track) */
	void SubscribeToFiredSlot(int32 TrackID);
	void UnsubscribeFromFiredSlot(int32 TrackID);

	/** Subscribe to track output meter level */
	void SubscribeToTrackOutputMeter(int32 TrackID);
	void UnsubscribeFromTrackOutputMeter(int32 TrackID);

	// === Scene Subscriptions ===

	/** Subscribe to scene triggered state (1-based SceneID) */
	void SubscribeToSceneTriggered(int32 SceneID);
	void UnsubscribeFromSceneTriggered(int32 SceneID);

	// === Device Subscriptions ===

	/** Subscribe to a specific device parameter value (all 1-based) */
	void SubscribeToDeviceParameter(int32 TrackID, int32 DeviceID, int32 ParameterID);
	void UnsubscribeFromDeviceParameter(int32 TrackID, int32 DeviceID, int32 ParameterID);

	// === Generic Subscription ===

	/**
	 * Subscribe to any AbletonOSC property.
	 * @param Category Object category: "song", "track", "clip", "scene", "device"
	 * @param Property The property name (e.g. "volume", "mute", "tempo")
	 * @param Indices Optional OSC indices (0-based). For track: {track_index}, for clip: {track_index, clip_index}
	 */
	void SubscribeToCustomProperty(const FString& Category, const FString& Property, const TArray<int32>& Indices);
	void UnsubscribeFromCustomProperty(const FString& Category, const FString& Property, const TArray<int32>& Indices);

	/** Unsubscribe from all active listeners at once */
	void UnsubscribeAll();

private:
	// OSC Message handler (called by UOSCServer)
	UFUNCTION()
	void HandleOSCMessage(const FOSCMessage& Message, const FString& IPAddress, int32 Port);

	// Internal helpers
	void SendListenCommand(const FString& StartOrStop, const FString& Category, const FString& Property, const TArray<int32>& Indices);
	FString BuildListenerKey(const FString& Category, const FString& Property, const TArray<int32>& Indices) const;

	// Parse OSC arguments from received message
	static bool GetIntArg(const FOSCMessage& Message, int32 Index, int32& OutValue);
	static bool GetFloatArg(const FOSCMessage& Message, int32 Index, float& OutValue);

	// State
	UPROPERTY()
	TObjectPtr<UOSCServer> OSCServer;

	UMoxAbletonSubsystem* OwnerSubsystem;
	bool bIsListening;
	int32 ListenPort;

	// Track which subscriptions are active (for UnsubscribeAll and cleanup)
	TSet<FString> ActiveSubscriptions;
};
