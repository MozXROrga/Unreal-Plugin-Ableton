# MoxAbleton - Technical Documentation

## Overview

MoxAbleton provides real-time control of Ableton Live from Unreal Engine via the AbletonOSC protocol. The plugin sends OSC commands to AbletonOSC (a MIDI Remote Script running inside Ableton Live) which translates them into Live API calls.

---

## Architecture

### Plugin Structure

```
MoxAbleton/
├── MoxAbleton.uplugin
├── README.md
├── DOCUMENTATION.md
├── CHANGELOG.md
└── Source/MoxAbleton/
    ├── MoxAbleton.Build.cs
    ├── Public/
    │   ├── MoxAbleton.h              # Module interface
    │   ├── MoxAbletonTypes.h           # Log category, enums
    │   ├── MoxAbletonSubsystem.h       # GameInstance subsystem
    │   └── MoxAbletonComponent.h       # Actor component
    └── Private/
        ├── MoxAbleton.cpp              # Module implementation
        ├── MoxAbletonSubsystem.cpp     # Subsystem implementation
        └── MoxAbletonComponent.cpp     # Component implementation
```

### Class Hierarchy

| Class | Type | Purpose |
|-------|------|---------|
| `UMoxAbletonSubsystem` | GameInstanceSubsystem | OSC connection, command routing, INI config |
| `UMoxAbletonComponent` | UActorComponent | Per-actor Blueprint interface |
| `EMoxAbletonConnectionState` | UENUM | Connection status tracking |

### Data Flow

```
  Blueprint/C++ Call
        │
        ▼
  MoxAbletonComponent ─── validates IDs ──▶ MoxAbletonSubsystem
        │                                         │
        │                                         ▼
        │                                 Build OSC Packet
        │                                 (binary protocol)
        │                                         │
        │                                         ▼
        │                                 UDP Socket Send
        │                                 → <configured-address>:11000
        │                                         │
        └─────────────────────────────────────────┘
                                                  │
                                                  ▼
                                          AbletonOSC (MIDI Script)
                                                  │
                                                  ▼
                                           Ableton Live API
```

---

## OSC Protocol

### Binary Format

All messages follow the OSC 1.0 binary specification:

| Part | Encoding |
|------|----------|
| **Address** | Null-terminated string, padded to 4-byte boundary |
| **Type Tag** | `","` + type chars (`i`/`f`/`s`), padded to 4-byte boundary |
| **Arguments** | Each padded to 4-byte boundary |

### Type Tags

| Tag | Type | Size | Encoding |
|-----|------|------|----------|
| `i` | int32 | 4 bytes | Big-endian |
| `f` | float32 | 4 bytes | IEEE 754, big-endian |
| `s` | string | variable | Null-terminated, padded to 4-byte boundary |

### AbletonOSC Ports

| Direction | Port | Description |
|-----------|------|-------------|
| **Send** | 11000 | Commands from UE → AbletonOSC |
| **Receive** | 11001 | Responses from AbletonOSC → UE (future use) |

---

## INI Configuration Schema

Section: `[moxableton]` in `Project/Config/MozConfig.ini`

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `abletonIP` / `ip` | string | `<configured-address>` | Ableton machine IP |
| `abletonPort` / `port` | int | `11000` | AbletonOSC listen port |
| `enabled` / `autostart` | bool | `false` | Auto-start on Initialize |
| `logCommands` | bool | `true` | Log fire/stop/set commands |
| `logOSCMessages` | bool | `false` | Log raw OSC packets (verbose) |
| `logCustomEvents` | bool | `true` | Log custom event calls |

### Example Config

```ini
[moxableton]
abletonIP=<configured-address>
abletonPort=11000
enabled=true
logCommands=true
logOSCMessages=false
logCustomEvents=true
```

---

## MoxAbletonSubsystem API

### System Control

| Function | Category | Description |
|----------|----------|-------------|
| `StartAbletonSystem()` | `MozXR\|Ableton` | Create socket, enable sending |
| `StopAbletonSystem()` | `MozXR\|Ableton` | Destroy socket, disable sending |
| `IsAbletonSystemRunning()` | `MozXR\|Ableton` | Returns `true` if enabled |
| `GetConnectionState()` | `MozXR\|Ableton` | Returns `EMoxAbletonConnectionState` |
| `SetAbletonAddress(IP, Port)` | `MozXR\|Ableton` | Change target, reconnects if running |
| `LoadConfigurationFromINI(bAutoStart)` | `MozXR\|Ableton` | Reload INI settings |

### Song Control

| Function | OSC Address | Category |
|----------|-------------|----------|
| `StartSong()` | `/live/song/start_playing` | `MozXR\|Ableton\|Song` |
| `StopSong()` | `/live/song/stop_playing` | `MozXR\|Ableton\|Song` |
| `StopAllClips()` | `/live/song/stop_all_clips` | `MozXR\|Ableton` |

### Scene Control

| Function | OSC Address | Category |
|----------|-------------|----------|
| `FireScene(SceneID)` | `/live/scene/fire [id]` | `MozXR\|Ableton\|Scene` |

### Clip Control

| Function | OSC Address | Category |
|----------|-------------|----------|
| `FireClip(TrackID, ClipSlot)` | `/live/clip/fire [t, s]` | `MozXR\|Ableton\|Clip` |
| `StopClip(TrackID, ClipSlot)` | `/live/clip/stop [t, s]` | `MozXR\|Ableton\|Clip` |

### Track Control

| Function | OSC Address | Category |
|----------|-------------|----------|
| `StopTrack(TrackID)` | `/live/track/stop_all_clips [id]` | `MozXR\|Ableton\|Track` |
| `SetTrackVolume(TrackID, Volume)` | `/live/track/set/volume [id, v]` | `MozXR\|Ableton\|Track` |
| `SetTrackMute(TrackID, bMute)` | `/live/track/set/mute [id, m]` | `MozXR\|Ableton\|Track` |
| `SetTrackPanning(TrackID, Panning)` | `/live/track/set/panning [id, p]` | `MozXR\|Ableton\|Track` |
| `SetTrackSolo(TrackID, bSolo)` | `/live/track/set/solo [id, s]` | `MozXR\|Ableton\|Track` |

### Device Control

| Function | OSC Address | Category |
|----------|-------------|----------|
| `SetDeviceParameter(T, D, P, V)` | `/live/device/set/parameter/value [t,d,p,v]` | `MozXR\|Ableton\|Device` |

### Custom Events

| Function | Category |
|----------|----------|
| `SendCustomEvent(EventPath, Parameters)` | `MozXR\|Ableton\|Advanced` |

**Type Auto-Detection:**

| Input Format | Detected Type | OSC Tag |
|--------------|---------------|---------|
| `"42"` | Integer | `i` |
| `"0.75"` | Float | `f` |
| `"MyClip"` | String | `s` |

---

## MoxAbletonComponent API

### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `AbletonTrackID` | int32 | 0 | Track number (1-based as in Ableton, 0 = disabled) |
| `AbletonClipSlot` | int32 | 0 | Clip slot number (1-based as in Ableton, 0 = disabled) |
| `AbletonSceneID` | int32 | 0 | Scene number (1-based as in Ableton, 0 = disabled) |

### Functions

All component functions use the configured properties. If a required property is 0 (disabled), a warning is logged and the call is skipped.

| Function | Required Properties | Category |
|----------|--------------------|----------|
| `FireClip()` | TrackID + ClipSlot | `MozXR\|Ableton\|Clip` |
| `StopClip()` | TrackID + ClipSlot | `MozXR\|Ableton\|Clip` |
| `FireScene()` | SceneID | `MozXR\|Ableton\|Scene` |
| `StopTrack()` | TrackID | `MozXR\|Ableton\|Track` |
| `SetTrackVolume(Volume)` | TrackID | `MozXR\|Ableton\|Track` |
| `SetTrackMute(bMute)` | TrackID | `MozXR\|Ableton\|Track` |
| `StartSong()` | none | `MozXR\|Ableton\|Song` |
| `StopSong()` | none | `MozXR\|Ableton\|Song` |
| `SendCustomEvent(Path, Params)` | none | `MozXR\|Ableton\|Advanced` |
| `IsRegistered()` | none | `MozXR\|Ableton` |

### Lifecycle

1. **BeginPlay**: Auto-registers with `UMoxAbletonSubsystem`
2. **Runtime**: Component functions delegate to subsystem with configured IDs
3. **EndPlay**: Auto-unregisters from subsystem

---

## Subsystem Lifecycle

```
Initialize()
  └─ LoadINISettings()
      └─ Parse [moxableton] from MozConfig.ini
      └─ If enabled=true → StartAbletonSystem()
                               └─ Create UDP Socket
                               └─ Set ConnectionState = Connected

Runtime
  └─ Components call Fire/Stop/Set functions
  └─ Functions build OSC packets → SendToAbleton()
  └─ UDP SendTo() → AbletonOSC

Deinitialize()
  └─ StopAllClips()    ← Emergency stop
  └─ Sleep(50ms)       ← Wait for OSC
  └─ StopAbletonSystem()
      └─ Close socket
      └─ Set ConnectionState = Disconnected
```

---

## Custom Events - Advanced Usage

The `SendCustomEvent()` function provides access to the **entire AbletonOSC API** without needing dedicated functions.

### Common Custom Events

| Event Path | Parameters | Description |
|------------|------------|-------------|
| `clip/set/name` | `[track, slot, "name"]` | Set clip name |
| `clip/set/color` | `[track, slot, color]` | Set clip color (int) |
| `clip/set/pitch_coarse` | `[track, slot, semitones]` | Pitch shift |
| `clip/set/gain` | `[track, slot, gain]` | Clip gain (float) |
| `track/set/name` | `[track, "name"]` | Set track name |
| `track/set/send` | `[track, send_idx, value]` | Set send level |
| `scene/set/name` | `[scene, "name"]` | Set scene name |
| `device/set/enabled` | `[track, device, "1"/"0"]` | Toggle device |

### When to Use Custom Events

- ✅ **Rare operations**: Changing names, colors, pitch
- ✅ **Experimental**: Testing new AbletonOSC features
- ✅ **Prototyping**: Quick testing without code changes
- ❌ **NOT for GET events**: Only SET/FIRE/STOP are supported

---

## Error Handling

### Common Errors

| Log Message | Cause | Solution |
|-------------|-------|----------|
| `Cannot send - socket not initialized` | System not started | Set `enabled=true` or call `StartAbletonSystem()` |
| `Cannot fire clip - Track or ClipSlot not set` | Property is 0 (disabled) | Set `AbletonTrackID` and `AbletonClipSlot` >= 1 |
| `Invalid Ableton IP` | Bad IP format | Check `abletonIP` in config |
| `Failed to create UDP socket` | System error | Check network/firewall |

### Log Category

All messages use `LogMoxAbleton`. Filter in Output Log or set verbosity:

```
log LogMoxAbleton Verbose
```

---

## Relationship to Other MozXR Plugins

| Plugin | Responsibility | Protocol |
|--------|---------------|----------|
| **MoxAbleton** | Ableton Live music control | AbletonOSC (Port 11000) |
| **MoxLisaBridge** | L-ISA spatial audio positioning | ADM-OSC (Port 9000) |
| **MoxAudio** | Legacy general audio (to be deactivated) | Custom OSC |

The plugins operate independently. MoxAbleton controls **what** plays, MoxLisaBridge controls **where** it sounds.

---

## AbletonOSC Reference

Full API documentation: [github.com/ideoforms/AbletonOSC](https://github.com/ideoforms/AbletonOSC)

### API Categories

| Category | Prefix | Example (0-based OSC indices) |
|----------|--------|-------------------------------|
| Song | `/live/song/` | `/live/song/start_playing` |
| Scene | `/live/scene/` | `/live/scene/fire [0]` (= Scene 1 in Ableton) |
| Clip | `/live/clip/` | `/live/clip/fire [0, 2]` (= Track 1, Slot 3) |
| Track | `/live/track/` | `/live/track/set/volume [0, 0.75]` (= Track 1) |
| Device | `/live/device/` | `/live/device/set/parameter/value [0,0,1,0.5]` |
| View | `/live/view/` | `/live/view/set/selected_track [0]` |

---

## Version

**MoxAbleton v1.1.2**

Compatible with Unreal Engine 5.7+  
Requires AbletonOSC (Ableton Live 10+)

