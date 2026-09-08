// Copyright Ars Electronica Futurelab. All Rights Reserved.

#include "MoxAbletonListener.h"
#include "MoxAbletonSubsystem.h"
#include "MoxAbletonTypes.h"

#include "OSCServer.h"
#include "OSCMessage.h"
#include "OSCManager.h"

// ============================================================================
// Constructor
// ============================================================================

UMoxAbletonListener::UMoxAbletonListener()
	: OSCServer(nullptr)
	, OwnerSubsystem(nullptr)
	, bIsListening(false)
	, ListenPort(11001)
{
}

// ============================================================================
// Lifecycle
// ============================================================================

void UMoxAbletonListener::Setup(UMoxAbletonSubsystem* InOwnerSubsystem)
{
	OwnerSubsystem = InOwnerSubsystem;
}

void UMoxAbletonListener::StartListening(int32 Port)
{
	if (bIsListening)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: Listener already running on port %d"), ListenPort);
		return;
	}

	ListenPort = Port;

	// Create UE5 OSC Server
	OSCServer = UOSCManager::CreateOSCServer(
		TEXT("0.0.0.0"),
		ListenPort,
		false, // bMulticastLoopback
		true,  // bStartListening
		FString(TEXT("MoxAbletonListener")),
		this
	);

	if (!OSCServer)
	{
		UE_LOG(LogMoxAbleton, Error, TEXT("MoxAbleton: Failed to create OSC Server on port %d"), ListenPort);
		return;
	}

	// Bind the message received delegate
	OSCServer->OnOscMessageReceived.AddDynamic(this, &UMoxAbletonListener::HandleOSCMessage);

	bIsListening = true;
	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Listener started on port %d"), ListenPort);
}

void UMoxAbletonListener::StopListening()
{
	if (!bIsListening)
	{
		return;
	}

	// Unsubscribe all active listeners in Ableton first
	UnsubscribeAll();

	// Stop and destroy server
	if (OSCServer)
	{
		OSCServer->OnOscMessageReceived.RemoveDynamic(this, &UMoxAbletonListener::HandleOSCMessage);
		OSCServer->Stop();
		OSCServer = nullptr;
	}

	bIsListening = false;
	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Listener stopped"));
}

// ============================================================================
// Song Subscriptions
// ============================================================================

void UMoxAbletonListener::SubscribeToBeat()
{
	TArray<int32> NoIndices;
	SendListenCommand(TEXT("start_listen"), TEXT("song"), TEXT("beat"), NoIndices);
}

void UMoxAbletonListener::UnsubscribeFromBeat()
{
	TArray<int32> NoIndices;
	SendListenCommand(TEXT("stop_listen"), TEXT("song"), TEXT("beat"), NoIndices);
}

void UMoxAbletonListener::SubscribeToTempo()
{
	TArray<int32> NoIndices;
	SendListenCommand(TEXT("start_listen"), TEXT("song"), TEXT("tempo"), NoIndices);
}

void UMoxAbletonListener::UnsubscribeFromTempo()
{
	TArray<int32> NoIndices;
	SendListenCommand(TEXT("stop_listen"), TEXT("song"), TEXT("tempo"), NoIndices);
}

void UMoxAbletonListener::SubscribeToIsPlaying()
{
	TArray<int32> NoIndices;
	SendListenCommand(TEXT("start_listen"), TEXT("song"), TEXT("is_playing"), NoIndices);
}

void UMoxAbletonListener::UnsubscribeFromIsPlaying()
{
	TArray<int32> NoIndices;
	SendListenCommand(TEXT("stop_listen"), TEXT("song"), TEXT("is_playing"), NoIndices);
}

void UMoxAbletonListener::SubscribeToCurrentSongTime()
{
	TArray<int32> NoIndices;
	SendListenCommand(TEXT("start_listen"), TEXT("song"), TEXT("current_song_time"), NoIndices);
}

void UMoxAbletonListener::UnsubscribeFromCurrentSongTime()
{
	TArray<int32> NoIndices;
	SendListenCommand(TEXT("stop_listen"), TEXT("song"), TEXT("current_song_time"), NoIndices);
}

// ============================================================================
// Track Subscriptions
// ============================================================================

void UMoxAbletonListener::SubscribeToTrackVolume(int32 TrackID)
{
	SendListenCommand(TEXT("start_listen"), TEXT("track"), TEXT("volume"), { TrackID - 1 });
}

void UMoxAbletonListener::UnsubscribeFromTrackVolume(int32 TrackID)
{
	SendListenCommand(TEXT("stop_listen"), TEXT("track"), TEXT("volume"), { TrackID - 1 });
}

void UMoxAbletonListener::SubscribeToTrackMute(int32 TrackID)
{
	SendListenCommand(TEXT("start_listen"), TEXT("track"), TEXT("mute"), { TrackID - 1 });
}

void UMoxAbletonListener::UnsubscribeFromTrackMute(int32 TrackID)
{
	SendListenCommand(TEXT("stop_listen"), TEXT("track"), TEXT("mute"), { TrackID - 1 });
}

void UMoxAbletonListener::SubscribeToTrackPanning(int32 TrackID)
{
	SendListenCommand(TEXT("start_listen"), TEXT("track"), TEXT("panning"), { TrackID - 1 });
}

void UMoxAbletonListener::UnsubscribeFromTrackPanning(int32 TrackID)
{
	SendListenCommand(TEXT("stop_listen"), TEXT("track"), TEXT("panning"), { TrackID - 1 });
}

void UMoxAbletonListener::SubscribeToTrackSolo(int32 TrackID)
{
	SendListenCommand(TEXT("start_listen"), TEXT("track"), TEXT("solo"), { TrackID - 1 });
}

void UMoxAbletonListener::UnsubscribeFromTrackSolo(int32 TrackID)
{
	SendListenCommand(TEXT("stop_listen"), TEXT("track"), TEXT("solo"), { TrackID - 1 });
}

void UMoxAbletonListener::SubscribeToPlayingSlot(int32 TrackID)
{
	SendListenCommand(TEXT("start_listen"), TEXT("track"), TEXT("playing_slot_index"), { TrackID - 1 });
}

void UMoxAbletonListener::UnsubscribeFromPlayingSlot(int32 TrackID)
{
	SendListenCommand(TEXT("stop_listen"), TEXT("track"), TEXT("playing_slot_index"), { TrackID - 1 });
}

void UMoxAbletonListener::SubscribeToFiredSlot(int32 TrackID)
{
	SendListenCommand(TEXT("start_listen"), TEXT("track"), TEXT("fired_slot_index"), { TrackID - 1 });
}

void UMoxAbletonListener::UnsubscribeFromFiredSlot(int32 TrackID)
{
	SendListenCommand(TEXT("stop_listen"), TEXT("track"), TEXT("fired_slot_index"), { TrackID - 1 });
}

void UMoxAbletonListener::SubscribeToTrackOutputMeter(int32 TrackID)
{
	SendListenCommand(TEXT("start_listen"), TEXT("track"), TEXT("output_meter_level"), { TrackID - 1 });
}

void UMoxAbletonListener::UnsubscribeFromTrackOutputMeter(int32 TrackID)
{
	SendListenCommand(TEXT("stop_listen"), TEXT("track"), TEXT("output_meter_level"), { TrackID - 1 });
}

// ============================================================================
// Scene Subscriptions
// ============================================================================

void UMoxAbletonListener::SubscribeToSceneTriggered(int32 SceneID)
{
	SendListenCommand(TEXT("start_listen"), TEXT("scene"), TEXT("is_triggered"), { SceneID - 1 });
}

void UMoxAbletonListener::UnsubscribeFromSceneTriggered(int32 SceneID)
{
	SendListenCommand(TEXT("stop_listen"), TEXT("scene"), TEXT("is_triggered"), { SceneID - 1 });
}

// ============================================================================
// Device Subscriptions
// ============================================================================

void UMoxAbletonListener::SubscribeToDeviceParameter(int32 TrackID, int32 DeviceID, int32 ParameterID)
{
	// Device listener uses a special path: /live/device/start_listen/parameter/value track device param
	FString Key = FString::Printf(TEXT("device.parameter/value.%d.%d.%d"), TrackID - 1, DeviceID - 1, ParameterID - 1);

	if (ActiveSubscriptions.Contains(Key))
	{
		return;
	}

	if (!OwnerSubsystem)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: Cannot subscribe - no subsystem reference"));
		return;
	}

	TArray<FString> Params;
	Params.Add(FString::FromInt(TrackID - 1));
	Params.Add(FString::FromInt(DeviceID - 1));
	Params.Add(FString::FromInt(ParameterID - 1));
	OwnerSubsystem->SendCustomEvent(TEXT("device/start_listen/parameter/value"), Params);

	ActiveSubscriptions.Add(Key);
	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Subscribed device/parameter/value [Track:%d, Device:%d, Param:%d]"), TrackID, DeviceID, ParameterID);
}

void UMoxAbletonListener::UnsubscribeFromDeviceParameter(int32 TrackID, int32 DeviceID, int32 ParameterID)
{
	FString Key = FString::Printf(TEXT("device.parameter/value.%d.%d.%d"), TrackID - 1, DeviceID - 1, ParameterID - 1);

	if (!ActiveSubscriptions.Contains(Key))
	{
		return;
	}

	if (OwnerSubsystem)
	{
		TArray<FString> Params;
		Params.Add(FString::FromInt(TrackID - 1));
		Params.Add(FString::FromInt(DeviceID - 1));
		Params.Add(FString::FromInt(ParameterID - 1));
		OwnerSubsystem->SendCustomEvent(TEXT("device/stop_listen/parameter/value"), Params);
	}

	ActiveSubscriptions.Remove(Key);
	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Unsubscribed device/parameter/value [Track:%d, Device:%d, Param:%d]"), TrackID, DeviceID, ParameterID);
}

// ============================================================================
// Generic Subscription
// ============================================================================

void UMoxAbletonListener::SubscribeToCustomProperty(const FString& Category, const FString& Property, const TArray<int32>& Indices)
{
	SendListenCommand(TEXT("start_listen"), Category, Property, Indices);
}

void UMoxAbletonListener::UnsubscribeFromCustomProperty(const FString& Category, const FString& Property, const TArray<int32>& Indices)
{
	SendListenCommand(TEXT("stop_listen"), Category, Property, Indices);
}

void UMoxAbletonListener::UnsubscribeAll()
{
	if (!OwnerSubsystem)
	{
		ActiveSubscriptions.Empty();
		return;
	}

	// Send stop_listen for each active subscription
	TArray<FString> KeysCopy = ActiveSubscriptions.Array();

	for (const FString& Key : KeysCopy)
	{
		// Parse key back into category.property.indices format
		FString Remaining = Key;
		FString Category, Property;

		int32 FirstDot;
		if (Remaining.FindChar(TEXT('.'), FirstDot))
		{
			Category = Remaining.Left(FirstDot);
			Remaining = Remaining.Mid(FirstDot + 1);
		}

		int32 SecondDot;
		if (Remaining.FindChar(TEXT('.'), SecondDot))
		{
			Property = Remaining.Left(SecondDot);
			Remaining = Remaining.Mid(SecondDot + 1);

			// Parse indices
			TArray<FString> IndexStrs;
			Remaining.ParseIntoArray(IndexStrs, TEXT("."));

			TArray<FString> Params;
			for (const FString& IdxStr : IndexStrs)
			{
				if (!IdxStr.IsEmpty())
				{
					Params.Add(IdxStr);
				}
			}

			FString EventPath = FString::Printf(TEXT("%s/stop_listen/%s"), *Category, *Property);
			OwnerSubsystem->SendCustomEvent(EventPath, Params);
		}
		else
		{
			// No indices (song-level properties)
			Property = Remaining;
			FString EventPath = FString::Printf(TEXT("%s/stop_listen/%s"), *Category, *Property);
			TArray<FString> EmptyParams;
			OwnerSubsystem->SendCustomEvent(EventPath, EmptyParams);
		}
	}

	int32 Count = ActiveSubscriptions.Num();
	ActiveSubscriptions.Empty();
	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Unsubscribed from all (%d listeners)"), Count);
}

// ============================================================================
// Internal: Send Listen Command
// ============================================================================

void UMoxAbletonListener::SendListenCommand(const FString& StartOrStop, const FString& Category, const FString& Property, const TArray<int32>& Indices)
{
	FString Key = BuildListenerKey(Category, Property, Indices);
	bool bIsStart = StartOrStop == TEXT("start_listen");

	if (bIsStart && ActiveSubscriptions.Contains(Key))
	{
		UE_LOG(LogMoxAbleton, Verbose, TEXT("MoxAbleton: Already subscribed: %s"), *Key);
		return;
	}

	if (!bIsStart && !ActiveSubscriptions.Contains(Key))
	{
		UE_LOG(LogMoxAbleton, Verbose, TEXT("MoxAbleton: Not subscribed: %s"), *Key);
		return;
	}

	if (!OwnerSubsystem)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: Cannot %s - no subsystem reference"), *StartOrStop);
		return;
	}

	// Build: category/start_listen/property with index params
	FString EventPath = FString::Printf(TEXT("%s/%s/%s"), *Category, *StartOrStop, *Property);

	TArray<FString> Params;
	for (int32 Idx : Indices)
	{
		Params.Add(FString::FromInt(Idx));
	}

	OwnerSubsystem->SendCustomEvent(EventPath, Params);

	if (bIsStart)
	{
		ActiveSubscriptions.Add(Key);
		UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Subscribed %s/%s %s"), *Category, *Property, Params.Num() > 0 ? *FString::Join(Params, TEXT(", ")) : TEXT(""));
	}
	else
	{
		ActiveSubscriptions.Remove(Key);
		UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Unsubscribed %s/%s"), *Category, *Property);
	}
}

FString UMoxAbletonListener::BuildListenerKey(const FString& Category, const FString& Property, const TArray<int32>& Indices) const
{
	FString Key = FString::Printf(TEXT("%s.%s"), *Category, *Property);
	for (int32 Idx : Indices)
	{
		Key += FString::Printf(TEXT(".%d"), Idx);
	}
	return Key;
}

// ============================================================================
// OSC Message Handler
// ============================================================================

bool UMoxAbletonListener::GetIntArg(const FOSCMessage& Message, int32 Index, int32& OutValue)
{
	return UOSCManager::GetInt32(Message, Index, OutValue);
}

bool UMoxAbletonListener::GetFloatArg(const FOSCMessage& Message, int32 Index, float& OutValue)
{
	return UOSCManager::GetFloat(Message, Index, OutValue);
}

void UMoxAbletonListener::HandleOSCMessage(const FOSCMessage& Message, const FString& IPAddress, int32 Port)
{
	if (!OwnerSubsystem)
	{
		return;
	}

	// Get the address as string
	FOSCAddress OscAddr = UOSCManager::GetOSCMessageAddress(Message);
	FString Address = UOSCManager::GetOSCAddressFullPath(OscAddr);

	UE_LOG(LogMoxAbleton, Verbose, TEXT("MoxAbleton: OSC Received: %s"), *Address);

	// ===== SONG EVENTS =====

	// /live/song/get/beat [int beat_number]
	if (Address == TEXT("/live/song/get/beat"))
	{
		int32 BeatNumber = 0;
		if (GetIntArg(Message, 0, BeatNumber))
		{
			OwnerSubsystem->OnBeatReceived.Broadcast(BeatNumber);
		}
		return;
	}

	// /live/song/get/tempo [float bpm]
	if (Address == TEXT("/live/song/get/tempo"))
	{
		float BPM = 0.0f;
		if (GetFloatArg(Message, 0, BPM))
		{
			OwnerSubsystem->OnTempoChanged.Broadcast(BPM);
		}
		return;
	}

	// /live/song/get/is_playing [int 0/1]
	if (Address == TEXT("/live/song/get/is_playing"))
	{
		int32 Playing = 0;
		if (GetIntArg(Message, 0, Playing))
		{
			OwnerSubsystem->OnPlayStateChanged.Broadcast(Playing != 0);
		}
		return;
	}

	// /live/song/get/current_song_time [float beats]
	if (Address == TEXT("/live/song/get/current_song_time"))
	{
		float SongTime = 0.0f;
		if (GetFloatArg(Message, 0, SongTime))
		{
			OwnerSubsystem->OnSongTimeChanged.Broadcast(SongTime);
		}
		return;
	}

	// ===== TRACK EVENTS =====
	// All track responses: /live/track/get/<property> [track_index, value]

	if (Address == TEXT("/live/track/get/volume"))
	{
		int32 TrackIndex = 0;
		float Value = 0.0f;
		if (GetIntArg(Message, 0, TrackIndex) && GetFloatArg(Message, 1, Value))
		{
			OwnerSubsystem->OnTrackVolumeChanged.Broadcast(TrackIndex + 1, Value);
		}
		return;
	}

	if (Address == TEXT("/live/track/get/panning"))
	{
		int32 TrackIndex = 0;
		float Value = 0.0f;
		if (GetIntArg(Message, 0, TrackIndex) && GetFloatArg(Message, 1, Value))
		{
			OwnerSubsystem->OnTrackPanningChanged.Broadcast(TrackIndex + 1, Value);
		}
		return;
	}

	if (Address == TEXT("/live/track/get/output_meter_level"))
	{
		int32 TrackIndex = 0;
		float Value = 0.0f;
		if (GetIntArg(Message, 0, TrackIndex) && GetFloatArg(Message, 1, Value))
		{
			OwnerSubsystem->OnTrackOutputMeterChanged.Broadcast(TrackIndex + 1, Value);
		}
		return;
	}

	if (Address == TEXT("/live/track/get/mute"))
	{
		int32 TrackIndex = 0;
		int32 Muted = 0;
		if (GetIntArg(Message, 0, TrackIndex) && GetIntArg(Message, 1, Muted))
		{
			OwnerSubsystem->OnTrackMuteChanged.Broadcast(TrackIndex + 1, Muted != 0);
		}
		return;
	}

	if (Address == TEXT("/live/track/get/solo"))
	{
		int32 TrackIndex = 0;
		int32 Soloed = 0;
		if (GetIntArg(Message, 0, TrackIndex) && GetIntArg(Message, 1, Soloed))
		{
			OwnerSubsystem->OnTrackSoloChanged.Broadcast(TrackIndex + 1, Soloed != 0);
		}
		return;
	}

	if (Address == TEXT("/live/track/get/playing_slot_index"))
	{
		int32 TrackIndex = 0;
		int32 SlotIndex = 0;
		if (GetIntArg(Message, 0, TrackIndex) && GetIntArg(Message, 1, SlotIndex))
		{
			// SlotIndex from Ableton is -1 if nothing is playing, otherwise 0-based
			OwnerSubsystem->OnPlayingSlotChanged.Broadcast(TrackIndex + 1, SlotIndex >= 0 ? SlotIndex + 1 : -1);
		}
		return;
	}

	if (Address == TEXT("/live/track/get/fired_slot_index"))
	{
		int32 TrackIndex = 0;
		int32 SlotIndex = 0;
		if (GetIntArg(Message, 0, TrackIndex) && GetIntArg(Message, 1, SlotIndex))
		{
			OwnerSubsystem->OnFiredSlotChanged.Broadcast(TrackIndex + 1, SlotIndex >= 0 ? SlotIndex + 1 : -1);
		}
		return;
	}

	// ===== SCENE EVENTS =====

	if (Address == TEXT("/live/scene/get/is_triggered"))
	{
		int32 SceneIndex = 0;
		int32 Triggered = 0;
		if (GetIntArg(Message, 0, SceneIndex) && GetIntArg(Message, 1, Triggered))
		{
			OwnerSubsystem->OnSceneTriggered.Broadcast(SceneIndex + 1, Triggered != 0);
		}
		return;
	}

	// ===== DEVICE EVENTS =====

	if (Address == TEXT("/live/device/get/parameter/value"))
	{
		int32 TrackIndex = 0, DeviceIndex = 0, ParamIndex = 0;
		float Value = 0.0f;
		if (GetIntArg(Message, 0, TrackIndex) &&
			GetIntArg(Message, 1, DeviceIndex) &&
			GetIntArg(Message, 2, ParamIndex) &&
			GetFloatArg(Message, 3, Value))
		{
			OwnerSubsystem->OnDeviceParameterChanged.Broadcast(TrackIndex + 1, DeviceIndex + 1, ParamIndex + 1, Value);
		}
		return;
	}

	// ===== CATCH-ALL =====
	// Fire generic delegate for any unhandled message
	{
		TArray<float> FloatArgs;
		TArray<int32> IntArgs;

		UOSCManager::GetAllFloats(Message, FloatArgs);
		UOSCManager::GetAllInt32s(Message, IntArgs);

		OwnerSubsystem->OnOSCReceived.Broadcast(Address, FloatArgs, IntArgs);
	}
}
