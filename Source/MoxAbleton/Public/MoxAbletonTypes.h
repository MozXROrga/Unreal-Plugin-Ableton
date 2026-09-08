// Copyright Ars Electronica Futurelab. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MoxAbletonTypes.generated.h"

// Log category for all MoxAbleton messages
DECLARE_LOG_CATEGORY_EXTERN(LogMoxAbleton, Log, All);

/**
 * Connection state of the Ableton Live bridge.
 */
UENUM(BlueprintType)
enum class EMoxAbletonConnectionState : uint8
{
	/** Not connected to Ableton Live */
	Disconnected	UMETA(DisplayName = "Disconnected"),
	/** Connected and communicating with Ableton Live */
	Connected		UMETA(DisplayName = "Connected")
};

// =============================================================================
// Listener Delegates (Receive from AbletonOSC)
// =============================================================================

/** Fired on every beat when subscribed via SubscribeToBeat() */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbletonBeat, int32, BeatNumber);

/** Fired when tempo changes in Ableton */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbletonTempoChanged, float, BPM);

/** Fired when play state changes (playing/stopped) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbletonPlayStateChanged, bool, bIsPlaying);

/** Fired when song position changes (in beats) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbletonSongTimeChanged, float, SongTimeBeats);

/** Fired for track float properties: volume, panning, output meter. TrackID is 1-based. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbletonTrackFloatChanged, int32, TrackID, float, Value);

/** Fired for track bool properties: mute, solo. TrackID is 1-based. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbletonTrackBoolChanged, int32, TrackID, bool, bValue);

/** Fired for track int properties: playing_slot_index, fired_slot_index. TrackID is 1-based. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbletonTrackIntChanged, int32, TrackID, int32, Value);

/** Fired when a scene triggered state changes. SceneID is 1-based. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbletonSceneTriggered, int32, SceneID, bool, bTriggered);

/** Fired for device parameter changes. All IDs are 1-based. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAbletonDeviceParameterChanged, int32, TrackID, int32, DeviceID, int32, ParameterID, float, Value);

/** Generic catch-all for any OSC message received from AbletonOSC */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAbletonOSCReceived, const FString&, Address, const TArray<float>&, FloatArgs, const TArray<int32>&, IntArgs);
