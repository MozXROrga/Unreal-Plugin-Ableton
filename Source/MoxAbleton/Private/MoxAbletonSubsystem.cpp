// Copyright Ars Electronica Futurelab. All Rights Reserved.

#include "MoxAbletonSubsystem.h"
#include "MoxAbletonComponent.h"
#include "MoxAbletonListener.h"
#include "MoxAbletonTypes.h"

#include "Common/UdpSocketBuilder.h"
#include "Misc/ConfigCacheIni.h"

#include "MoxUtility/Public/MoxUtilityBPLibrary.h"

// ============================================================================
// Constructor
// ============================================================================

UMoxAbletonSubsystem::UMoxAbletonSubsystem()
	: AbletonSocket(nullptr)
	, bSystemEnabled(false)
	, ConnectionState(EMoxAbletonConnectionState::Disconnected)
	, AbletonIP(TEXT("127.0.0.1"))
	, AbletonPort(11000)
	, ReceivePort(11001)
	, Listener(nullptr)
	, bLogCommands(true)
	, bLogOSCMessages(false)
	, bLogCustomEvents(true)
{
}

// ============================================================================
// Lifecycle
// ============================================================================

void UMoxAbletonSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Subsystem initializing..."));

	// Create listener object
	Listener = NewObject<UMoxAbletonListener>(this);
	Listener->Setup(this);

	LoadINISettings();
}

void UMoxAbletonSubsystem::Deinitialize()
{
	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Subsystem deinitializing..."));

	// Stop listener first (sends unsubscribe messages while socket is still open)
	if (Listener && Listener->IsListening())
	{
		Listener->StopListening();
	}

	// Clean shutdown: Stop song transport (preserves track/clip state)
	if (bSystemEnabled && AbletonSocket)
	{
		UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Stopping Ableton transport..."));
		StopSong();

		// Small delay to ensure OSC message is sent
		FPlatformProcess::Sleep(0.05f);
	}

	StopAbletonSystem();

	Super::Deinitialize();
}

// ============================================================================
// Configuration
// ============================================================================

void UMoxAbletonSubsystem::LoadINISettings()
{
	FString ConfigPath = UMoxUtilityBPLibrary::GetConfigFilePath();

	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Loading config from: %s"), *ConfigPath);

	if (!FPaths::FileExists(ConfigPath))
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: Config file not found: %s - using defaults"), *ConfigPath);
		return;
	}

	FString SectionName = TEXT("moxableton");
	FString FileContent;
	FFileHelper::LoadFileToString(FileContent, *ConfigPath);

	// Parse config values
	FString Value;
	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines);

	bool bInSection = false;

	for (const FString& Line : Lines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();

		// Check section header
		if (TrimmedLine.StartsWith(TEXT("[")))
		{
			FString SectionHeader = TrimmedLine.Mid(1);
			int32 CloseBracket = INDEX_NONE;
			SectionHeader.FindChar(TEXT(']'), CloseBracket);
			if (CloseBracket != INDEX_NONE)
			{
				SectionHeader = SectionHeader.Left(CloseBracket);
			}
			bInSection = SectionHeader.Equals(SectionName, ESearchCase::IgnoreCase);
			continue;
		}

		if (!bInSection || TrimmedLine.IsEmpty() || TrimmedLine.StartsWith(TEXT("//")))
		{
			continue;
		}

		// Parse key=value
		FString Key, Val;
		if (TrimmedLine.Split(TEXT("="), &Key, &Val))
		{
			Key = Key.TrimStartAndEnd().ToLower();
			Val = Val.TrimStartAndEnd();

			if (Key == TEXT("abletonip") || Key == TEXT("ip"))
			{
				AbletonIP = Val;
			}
			else if (Key == TEXT("abletonport") || Key == TEXT("port"))
			{
				AbletonPort = FCString::Atoi(*Val);
			}
			else if (Key == TEXT("enabled") || Key == TEXT("autostart"))
			{
				if (Val.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Val == TEXT("1"))
				{
					bSystemEnabled = false; // Will be started in StartAbletonSystem()
					// Auto-start after config load
					StartAbletonSystem();
				}
			}
			else if (Key == TEXT("logcommands"))
			{
				bLogCommands = Val.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Val == TEXT("1");
			}
			else if (Key == TEXT("logoscmessages"))
			{
				bLogOSCMessages = Val.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Val == TEXT("1");
			}
			else if (Key == TEXT("logcustomevents"))
			{
				bLogCustomEvents = Val.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Val == TEXT("1");
			}
			else if (Key == TEXT("receiveport") || Key == TEXT("listenport"))
			{
				ReceivePort = FCString::Atoi(*Val);
			}
		}
	}

	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Config loaded - IP: %s, Port: %d, ReceivePort: %d"), *AbletonIP, AbletonPort, ReceivePort);
}

void UMoxAbletonSubsystem::LoadConfigurationFromINI(bool bAutoStart)
{
	LoadINISettings();
	if (bAutoStart)
	{
		StartAbletonSystem();
	}
}

void UMoxAbletonSubsystem::SetAbletonAddress(const FString& IP, int32 Port)
{
	AbletonIP = IP;
	AbletonPort = Port;
	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Address set to %s:%d"), *AbletonIP, AbletonPort);

	// Recreate socket if system is running
	if (bSystemEnabled)
	{
		StopAbletonSystem();
		StartAbletonSystem();
	}
}

// ============================================================================
// System Control
// ============================================================================

void UMoxAbletonSubsystem::StartAbletonSystem()
{
	if (bSystemEnabled && AbletonSocket)
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: System already running"));
		return;
	}

	// Create UDP socket
	AbletonSocket = FUdpSocketBuilder(TEXT("MoxAbletonSocket"))
		.AsNonBlocking()
		.AsReusable()
		.Build();

	if (!AbletonSocket)
	{
		UE_LOG(LogMoxAbleton, Error, TEXT("MoxAbleton: Failed to create UDP socket"));
		ConnectionState = EMoxAbletonConnectionState::Disconnected;
		return;
	}

	bSystemEnabled = true;
	ConnectionState = EMoxAbletonConnectionState::Connected;

	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: System started -> %s:%d"), *AbletonIP, AbletonPort);
}

void UMoxAbletonSubsystem::StopAbletonSystem()
{
	bSystemEnabled = false;
	ConnectionState = EMoxAbletonConnectionState::Disconnected;

	if (AbletonSocket)
	{
		AbletonSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(AbletonSocket);
		AbletonSocket = nullptr;

		UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: System stopped"));
	}
}

// ============================================================================
// OSC Sending
// ============================================================================

/**
 * Core OSC send function. Constructs and sends a raw OSC packet to Ableton.
 *
 * OSC Binary Protocol:
 * - Address: null-terminated, padded to 4-byte boundary
 * - Type tag string: ",<types>" padded to 4-byte boundary
 * - Arguments: each padded to 4-byte boundary
 *   - 'i' (int32): 4 bytes big-endian
 *   - 'f' (float): 4 bytes IEEE754 big-endian
 *   - 's' (string): null-terminated, padded to 4-byte boundary
 */
void UMoxAbletonSubsystem::SendToAbleton(const FString& Address, const TArray<uint8>& PacketData, int32 PacketSize)
{
	if (!AbletonSocket || !bSystemEnabled)
	{
		if (bLogOSCMessages)
		{
			UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: Cannot send - socket not initialized or system disabled"));
		}
		return;
	}

	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool bIsValid = false;
	Addr->SetIp(*AbletonIP, bIsValid);

	if (!bIsValid)
	{
		UE_LOG(LogMoxAbleton, Error, TEXT("MoxAbleton: Invalid IP: %s"), *AbletonIP);
		return;
	}

	Addr->SetPort(AbletonPort);

	int32 BytesSent = 0;
	AbletonSocket->SendTo(PacketData.GetData(), PacketSize, BytesSent, *Addr);

	if (bLogOSCMessages)
	{
		UE_LOG(LogMoxAbleton, Verbose, TEXT("MoxAbleton: OSC -> %s (%d bytes)"), *Address, BytesSent);
	}
}

// ============================================================================
// OSC Packet Construction Helpers
// ============================================================================

namespace MoxAbletonOSC
{
	// Pad to 4-byte boundary
	static int32 PadToFour(int32 Length)
	{
		return (Length + 3) & ~3;
	}

	// Write a null-terminated string padded to 4-byte boundary
	static int32 WriteString(uint8* Buffer, int32 Offset, const FString& Str)
	{
		auto Utf8 = StringCast<ANSICHAR>(*Str);
		const ANSICHAR* Data = Utf8.Get();
		int32 Len = FCStringAnsi::Strlen(Data) + 1; // include null
		int32 PaddedLen = PadToFour(Len);

		FMemory::Memcpy(Buffer + Offset, Data, Len);
		// Zero-fill padding
		for (int32 i = Len; i < PaddedLen; ++i)
		{
			Buffer[Offset + i] = 0;
		}
		return PaddedLen;
	}

	// Write int32 big-endian
	static int32 WriteInt32(uint8* Buffer, int32 Offset, int32 Value)
	{
		Buffer[Offset + 0] = (Value >> 24) & 0xFF;
		Buffer[Offset + 1] = (Value >> 16) & 0xFF;
		Buffer[Offset + 2] = (Value >> 8) & 0xFF;
		Buffer[Offset + 3] = Value & 0xFF;
		return 4;
	}

	// Write float big-endian
	static int32 WriteFloat(uint8* Buffer, int32 Offset, float Value)
	{
		uint32 IntVal;
		FMemory::Memcpy(&IntVal, &Value, 4);
		Buffer[Offset + 0] = (IntVal >> 24) & 0xFF;
		Buffer[Offset + 1] = (IntVal >> 16) & 0xFF;
		Buffer[Offset + 2] = (IntVal >> 8) & 0xFF;
		Buffer[Offset + 3] = IntVal & 0xFF;
		return 4;
	}

	// Build and return a complete OSC packet for a command with no arguments
	static TArray<uint8> BuildPacketNoArgs(const FString& Address, int32& OutSize)
	{
		TArray<uint8> Packet;
		Packet.SetNumZeroed(1024);

		int32 Offset = WriteString(Packet.GetData(), 0, Address);
		Offset += WriteString(Packet.GetData(), Offset, TEXT(","));

		OutSize = Offset;
		return Packet;
	}

	// Build a packet with one int argument
	static TArray<uint8> BuildPacketOneInt(const FString& Address, int32 Arg, int32& OutSize)
	{
		TArray<uint8> Packet;
		Packet.SetNumZeroed(1024);

		int32 Offset = WriteString(Packet.GetData(), 0, Address);
		Offset += WriteString(Packet.GetData(), Offset, TEXT(",i"));
		Offset += WriteInt32(Packet.GetData(), Offset, Arg);

		OutSize = Offset;
		return Packet;
	}

	// Build a packet with two int arguments
	static TArray<uint8> BuildPacketTwoInts(const FString& Address, int32 Arg1, int32 Arg2, int32& OutSize)
	{
		TArray<uint8> Packet;
		Packet.SetNumZeroed(1024);

		int32 Offset = WriteString(Packet.GetData(), 0, Address);
		Offset += WriteString(Packet.GetData(), Offset, TEXT(",ii"));
		Offset += WriteInt32(Packet.GetData(), Offset, Arg1);
		Offset += WriteInt32(Packet.GetData(), Offset, Arg2);

		OutSize = Offset;
		return Packet;
	}

	// Build a packet with int + float arguments
	static TArray<uint8> BuildPacketIntFloat(const FString& Address, int32 IntArg, float FloatArg, int32& OutSize)
	{
		TArray<uint8> Packet;
		Packet.SetNumZeroed(1024);

		int32 Offset = WriteString(Packet.GetData(), 0, Address);
		Offset += WriteString(Packet.GetData(), Offset, TEXT(",if"));
		Offset += WriteInt32(Packet.GetData(), Offset, IntArg);
		Offset += WriteFloat(Packet.GetData(), Offset, FloatArg);

		OutSize = Offset;
		return Packet;
	}

	// Build a packet with int + int + float arguments
	static TArray<uint8> BuildPacketTwoIntsFloat(const FString& Address, int32 Int1, int32 Int2, float FloatArg, int32& OutSize)
	{
		TArray<uint8> Packet;
		Packet.SetNumZeroed(1024);

		int32 Offset = WriteString(Packet.GetData(), 0, Address);
		Offset += WriteString(Packet.GetData(), Offset, TEXT(",iif"));
		Offset += WriteInt32(Packet.GetData(), Offset, Int1);
		Offset += WriteInt32(Packet.GetData(), Offset, Int2);
		Offset += WriteFloat(Packet.GetData(), Offset, FloatArg);

		OutSize = Offset;
		return Packet;
	}
} // namespace MoxAbletonOSC

// ============================================================================
// Song Control
// ============================================================================

void UMoxAbletonSubsystem::StartSong()
{
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Song -> Start"));

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketNoArgs(TEXT("/live/song/start_playing"), Size);
	SendToAbleton(TEXT("/live/song/start_playing"), Packet, Size);
}

void UMoxAbletonSubsystem::StopSong()
{
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Song -> Stop"));

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketNoArgs(TEXT("/live/song/stop_playing"), Size);
	SendToAbleton(TEXT("/live/song/stop_playing"), Packet, Size);
}

void UMoxAbletonSubsystem::StopAllClips()
{
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: EMERGENCY - Stop ALL clips"));

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketNoArgs(TEXT("/live/song/stop_all_clips"), Size);
	SendToAbleton(TEXT("/live/song/stop_all_clips"), Packet, Size);
}

// ============================================================================
// Scene Control
// ============================================================================

void UMoxAbletonSubsystem::FireScene(int32 SceneID)
{
	// Convert from 1-based (Ableton UI) to 0-based (OSC API)
	int32 OSCSceneIndex = SceneID - 1;
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Scene %d (OSC:%d) -> Fire"), SceneID, OSCSceneIndex);

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketOneInt(TEXT("/live/scene/fire"), OSCSceneIndex, Size);
	SendToAbleton(TEXT("/live/scene/fire"), Packet, Size);
}

// ============================================================================
// Clip Control
// ============================================================================

void UMoxAbletonSubsystem::FireClip(int32 TrackID, int32 ClipSlot)
{
	// Convert from 1-based (Ableton UI) to 0-based (OSC API)
	int32 OSCTrack = TrackID - 1;
	int32 OSCSlot = ClipSlot - 1;
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Clip [Track:%d, Slot:%d] (OSC:%d,%d) -> Fire"), TrackID, ClipSlot, OSCTrack, OSCSlot);

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketTwoInts(TEXT("/live/clip/fire"), OSCTrack, OSCSlot, Size);
	SendToAbleton(TEXT("/live/clip/fire"), Packet, Size);
}

void UMoxAbletonSubsystem::StopClip(int32 TrackID, int32 ClipSlot)
{
	// Convert from 1-based (Ableton UI) to 0-based (OSC API)
	int32 OSCTrack = TrackID - 1;
	int32 OSCSlot = ClipSlot - 1;
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Clip [Track:%d, Slot:%d] (OSC:%d,%d) -> Stop"), TrackID, ClipSlot, OSCTrack, OSCSlot);

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketTwoInts(TEXT("/live/clip/stop"), OSCTrack, OSCSlot, Size);
	SendToAbleton(TEXT("/live/clip/stop"), Packet, Size);
}

// ============================================================================
// Track Control
// ============================================================================

void UMoxAbletonSubsystem::StopTrack(int32 TrackID)
{
	// Convert from 1-based (Ableton UI) to 0-based (OSC API)
	int32 OSCTrack = TrackID - 1;
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Track %d (OSC:%d) -> Stop"), TrackID, OSCTrack);

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketOneInt(TEXT("/live/track/stop_all_clips"), OSCTrack, Size);
	SendToAbleton(TEXT("/live/track/stop_all_clips"), Packet, Size);
}

void UMoxAbletonSubsystem::SetTrackVolume(int32 TrackID, float Volume)
{
	// Convert from 1-based (Ableton UI) to 0-based (OSC API)
	int32 OSCTrack = TrackID - 1;
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Track %d (OSC:%d) -> Volume: %.2f"), TrackID, OSCTrack, Volume);

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketIntFloat(TEXT("/live/track/set/volume"), OSCTrack, Volume, Size);
	SendToAbleton(TEXT("/live/track/set/volume"), Packet, Size);
}

void UMoxAbletonSubsystem::SetTrackMute(int32 TrackID, bool bMute)
{
	// Convert from 1-based (Ableton UI) to 0-based (OSC API)
	int32 OSCTrack = TrackID - 1;
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Track %d (OSC:%d) -> Mute: %s"), TrackID, OSCTrack, bMute ? TEXT("ON") : TEXT("OFF"));

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketIntFloat(TEXT("/live/track/set/mute"), OSCTrack, bMute ? 1.0f : 0.0f, Size);
	SendToAbleton(TEXT("/live/track/set/mute"), Packet, Size);
}

void UMoxAbletonSubsystem::SetTrackPanning(int32 TrackID, float Panning)
{
	// Convert from 1-based (Ableton UI) to 0-based (OSC API)
	int32 OSCTrack = TrackID - 1;
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Track %d (OSC:%d) -> Panning: %.2f"), TrackID, OSCTrack, Panning);

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketIntFloat(TEXT("/live/track/set/panning"), OSCTrack, Panning, Size);
	SendToAbleton(TEXT("/live/track/set/panning"), Packet, Size);
}

void UMoxAbletonSubsystem::SetTrackSolo(int32 TrackID, bool bSolo)
{
	// Convert from 1-based (Ableton UI) to 0-based (OSC API)
	int32 OSCTrack = TrackID - 1;
	if (bLogCommands) UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Track %d (OSC:%d) -> Solo: %s"), TrackID, OSCTrack, bSolo ? TEXT("ON") : TEXT("OFF"));

	int32 Size;
	TArray<uint8> Packet = MoxAbletonOSC::BuildPacketIntFloat(TEXT("/live/track/set/solo"), OSCTrack, bSolo ? 1.0f : 0.0f, Size);
	SendToAbleton(TEXT("/live/track/set/solo"), Packet, Size);
}

// ============================================================================
// Device Control
// ============================================================================

void UMoxAbletonSubsystem::SetDeviceParameter(int32 TrackID, int32 DeviceID, int32 ParameterID, float Value)
{
	// Convert from 1-based (Ableton UI) to 0-based (OSC API)
	int32 OSCTrack = TrackID - 1;
	int32 OSCDevice = DeviceID - 1;
	int32 OSCParam = ParameterID - 1;
	if (bLogCommands)
	{
		UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Device [Track:%d, Device:%d, Param:%d] (OSC:%d,%d,%d) -> %.2f"),
			TrackID, DeviceID, ParameterID, OSCTrack, OSCDevice, OSCParam, Value);
	}

	// This command needs 3 ints + 1 float
	TArray<uint8> FullPacket;
	FullPacket.SetNumZeroed(1024);
	int32 Offset = MoxAbletonOSC::WriteString(FullPacket.GetData(), 0, TEXT("/live/device/set/parameter/value"));
	Offset += MoxAbletonOSC::WriteString(FullPacket.GetData(), Offset, TEXT(",iiif"));
	Offset += MoxAbletonOSC::WriteInt32(FullPacket.GetData(), Offset, OSCTrack);
	Offset += MoxAbletonOSC::WriteInt32(FullPacket.GetData(), Offset, OSCDevice);
	Offset += MoxAbletonOSC::WriteInt32(FullPacket.GetData(), Offset, OSCParam);
	Offset += MoxAbletonOSC::WriteFloat(FullPacket.GetData(), Offset, Value);

	SendToAbleton(TEXT("/live/device/set/parameter/value"), FullPacket, Offset);
}

// ============================================================================
// Custom Events
// ============================================================================

void UMoxAbletonSubsystem::SendCustomEvent(const FString& EventPath, const TArray<FString>& Parameters)
{
	FString FullAddress = FString::Printf(TEXT("/live/%s"), *EventPath);

	if (bLogCustomEvents)
	{
		FString ParamStr;
		for (const FString& P : Parameters)
		{
			if (!ParamStr.IsEmpty()) ParamStr += TEXT(", ");
			ParamStr += P;
		}
		UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Custom Event -> %s [%s]"), *FullAddress, *ParamStr);
	}

	// Build type tag string and argument data
	FString TypeTag = TEXT(",");
	TArray<uint8> ArgData;
	ArgData.SetNumZeroed(2048);
	int32 ArgOffset = 0;

	for (const FString& Param : Parameters)
	{
		// Auto-detect type
		if (Param.IsNumeric())
		{
			if (Param.Contains(TEXT(".")))
			{
				// Float
				float FloatVal = FCString::Atof(*Param);
				TypeTag += TEXT("f");
				ArgOffset += MoxAbletonOSC::WriteFloat(ArgData.GetData(), ArgOffset, FloatVal);
			}
			else
			{
				// Integer
				int32 IntVal = FCString::Atoi(*Param);
				TypeTag += TEXT("i");
				ArgOffset += MoxAbletonOSC::WriteInt32(ArgData.GetData(), ArgOffset, IntVal);
			}
		}
		else
		{
			// String
			TypeTag += TEXT("s");
			ArgOffset += MoxAbletonOSC::WriteString(ArgData.GetData(), ArgOffset, Param);
		}
	}

	// Build final packet
	TArray<uint8> Packet;
	Packet.SetNumZeroed(4096);
	int32 Offset = MoxAbletonOSC::WriteString(Packet.GetData(), 0, FullAddress);
	Offset += MoxAbletonOSC::WriteString(Packet.GetData(), Offset, TypeTag);

	// Copy arg data
	FMemory::Memcpy(Packet.GetData() + Offset, ArgData.GetData(), ArgOffset);
	Offset += ArgOffset;

	SendToAbleton(FullAddress, Packet, Offset);
}

// ============================================================================
// Component Registration
// ============================================================================

bool UMoxAbletonSubsystem::RegisterComponent(UMoxAbletonComponent* Component)
{
	if (!Component) return false;

	FString Name = Component->GetOwner() ? Component->GetOwner()->GetName() : TEXT("Unknown");

	if (RegisteredComponents.Contains(Name))
	{
		UE_LOG(LogMoxAbleton, Warning, TEXT("MoxAbleton: Component '%s' already registered"), *Name);
		return false;
	}

	RegisteredComponents.Add(Name, Component);
	UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Component registered: %s"), *Name);
	return true;
}

void UMoxAbletonSubsystem::UnregisterComponent(UMoxAbletonComponent* Component)
{
	if (!Component) return;

	FString Name = Component->GetOwner() ? Component->GetOwner()->GetName() : TEXT("Unknown");

	if (RegisteredComponents.Remove(Name) > 0)
	{
		UE_LOG(LogMoxAbleton, Log, TEXT("MoxAbleton: Component unregistered: %s"), *Name);
	}
}

// ============================================================================
// Listener (Receive from AbletonOSC)
// ============================================================================

void UMoxAbletonSubsystem::StartListening()
{
	if (!Listener)
	{
		UE_LOG(LogMoxAbleton, Error, TEXT("MoxAbleton: Listener not initialized"));
		return;
	}

	Listener->StartListening(ReceivePort);
}

void UMoxAbletonSubsystem::StopListening()
{
	if (!Listener) return;
	Listener->StopListening();
}

bool UMoxAbletonSubsystem::IsListening() const
{
	return Listener && Listener->IsListening();
}

// --- Song Subscribe Wrappers ---

void UMoxAbletonSubsystem::SubscribeToBeat() { if (Listener) Listener->SubscribeToBeat(); }
void UMoxAbletonSubsystem::UnsubscribeFromBeat() { if (Listener) Listener->UnsubscribeFromBeat(); }
void UMoxAbletonSubsystem::SubscribeToTempo() { if (Listener) Listener->SubscribeToTempo(); }
void UMoxAbletonSubsystem::UnsubscribeFromTempo() { if (Listener) Listener->UnsubscribeFromTempo(); }
void UMoxAbletonSubsystem::SubscribeToIsPlaying() { if (Listener) Listener->SubscribeToIsPlaying(); }
void UMoxAbletonSubsystem::UnsubscribeFromIsPlaying() { if (Listener) Listener->UnsubscribeFromIsPlaying(); }
void UMoxAbletonSubsystem::SubscribeToCurrentSongTime() { if (Listener) Listener->SubscribeToCurrentSongTime(); }
void UMoxAbletonSubsystem::UnsubscribeFromCurrentSongTime() { if (Listener) Listener->UnsubscribeFromCurrentSongTime(); }

// --- Track Subscribe Wrappers ---

void UMoxAbletonSubsystem::SubscribeToTrackVolume(int32 TrackID) { if (Listener) Listener->SubscribeToTrackVolume(TrackID); }
void UMoxAbletonSubsystem::UnsubscribeFromTrackVolume(int32 TrackID) { if (Listener) Listener->UnsubscribeFromTrackVolume(TrackID); }
void UMoxAbletonSubsystem::SubscribeToTrackMute(int32 TrackID) { if (Listener) Listener->SubscribeToTrackMute(TrackID); }
void UMoxAbletonSubsystem::UnsubscribeFromTrackMute(int32 TrackID) { if (Listener) Listener->UnsubscribeFromTrackMute(TrackID); }
void UMoxAbletonSubsystem::SubscribeToTrackPanning(int32 TrackID) { if (Listener) Listener->SubscribeToTrackPanning(TrackID); }
void UMoxAbletonSubsystem::UnsubscribeFromTrackPanning(int32 TrackID) { if (Listener) Listener->UnsubscribeFromTrackPanning(TrackID); }
void UMoxAbletonSubsystem::SubscribeToTrackSolo(int32 TrackID) { if (Listener) Listener->SubscribeToTrackSolo(TrackID); }
void UMoxAbletonSubsystem::UnsubscribeFromTrackSolo(int32 TrackID) { if (Listener) Listener->UnsubscribeFromTrackSolo(TrackID); }
void UMoxAbletonSubsystem::SubscribeToPlayingSlot(int32 TrackID) { if (Listener) Listener->SubscribeToPlayingSlot(TrackID); }
void UMoxAbletonSubsystem::UnsubscribeFromPlayingSlot(int32 TrackID) { if (Listener) Listener->UnsubscribeFromPlayingSlot(TrackID); }
void UMoxAbletonSubsystem::SubscribeToFiredSlot(int32 TrackID) { if (Listener) Listener->SubscribeToFiredSlot(TrackID); }
void UMoxAbletonSubsystem::UnsubscribeFromFiredSlot(int32 TrackID) { if (Listener) Listener->UnsubscribeFromFiredSlot(TrackID); }
void UMoxAbletonSubsystem::SubscribeToTrackOutputMeter(int32 TrackID) { if (Listener) Listener->SubscribeToTrackOutputMeter(TrackID); }
void UMoxAbletonSubsystem::UnsubscribeFromTrackOutputMeter(int32 TrackID) { if (Listener) Listener->UnsubscribeFromTrackOutputMeter(TrackID); }

// --- Scene Subscribe Wrappers ---

void UMoxAbletonSubsystem::SubscribeToSceneTriggered(int32 SceneID) { if (Listener) Listener->SubscribeToSceneTriggered(SceneID); }
void UMoxAbletonSubsystem::UnsubscribeFromSceneTriggered(int32 SceneID) { if (Listener) Listener->UnsubscribeFromSceneTriggered(SceneID); }

// --- Device Subscribe Wrappers ---

void UMoxAbletonSubsystem::SubscribeToDeviceParameter(int32 TrackID, int32 DeviceID, int32 ParameterID) { if (Listener) Listener->SubscribeToDeviceParameter(TrackID, DeviceID, ParameterID); }
void UMoxAbletonSubsystem::UnsubscribeFromDeviceParameter(int32 TrackID, int32 DeviceID, int32 ParameterID) { if (Listener) Listener->UnsubscribeFromDeviceParameter(TrackID, DeviceID, ParameterID); }

// --- Generic ---

void UMoxAbletonSubsystem::SubscribeToCustomProperty(const FString& Category, const FString& Property, const TArray<int32>& Indices) { if (Listener) Listener->SubscribeToCustomProperty(Category, Property, Indices); }
void UMoxAbletonSubsystem::UnsubscribeFromCustomProperty(const FString& Category, const FString& Property, const TArray<int32>& Indices) { if (Listener) Listener->UnsubscribeFromCustomProperty(Category, Property, Indices); }
void UMoxAbletonSubsystem::UnsubscribeAll() { if (Listener) Listener->UnsubscribeAll(); }

