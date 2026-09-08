# Changelog

All notable changes to the MoxAbleton plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## Unreleased

## [3.0.0] - 2026-08-05

- Standardized the English documentation set and release status.
- Aligned the suite version without runtime API changes.

## [Unreleased]

### Planned Features
- OSC receive handler (port 11001) for Ableton responses
- Connection health monitoring / heartbeat
- Beat synchronization delegates
- Scene name query and mapping
- Tempo and time signature control

## [1.1.2] - 2026-08-05

### Changed
- Declared MoxUtility as the shared source of the active `MozConfig.ini` path.
- Updated the Ableton example Blueprints and maps.

---

## [1.1.1] - 2026-02-10

### Removed

#### StartTrack() removed
- **`StartTrack()` / `StartTrack(TrackID)` removed** — AbletonOSC has no `/live/track/play` endpoint. The only Track method is `/live/track/stop_all_clips`.
- Use `FireClip(TrackID, ClipSlot)` or `FireScene(SceneID)` to start playback instead.

---

## [1.1.0] - 2026-02-09

### Changed

#### 1-Based Indexing (UX Fix)
- **All Track/Clip/Scene IDs now use 1-based numbering**, matching Ableton Live's visual display
- Previously: `TrackID=0` → Track 1 in Ableton (confusing)
- Now: `TrackID=1` → Track 1 in Ableton (consistent)
- `0` = disabled (was `-1`)
- Subsystem internally converts to 0-based OSC indices (`-1` offset)
- Log messages now show both user-facing number and OSC index: `Track 2 (OSC:1) → Fire`
- `SendCustomEvent()` is **not affected** — advanced users pass raw 0-based OSC parameters directly

#### Component Properties
- `AbletonTrackID`: default `0` (disabled), `ClampMin=0`, 1-based
- `AbletonClipSlot`: default `0` (disabled), `ClampMin=0`, 1-based
- `AbletonSceneID`: default `0` (disabled), `ClampMin=0`, 1-based
- Validation guards updated from `< 0` to `< 1`

#### Removed
- `StopScene()` / `StopScene(SceneID)` — AbletonOSC has no scene stop endpoint (`/live/scene/stop_all_clips` does not exist in the API)

---

## [1.0.0] - 2026-02-09

### Added

#### Core Plugin
- **MoxAbletonSubsystem**: GameInstance subsystem for Ableton Live OSC communication
- **MoxAbletonComponent**: Blueprint-spawnable actor component for per-actor Ableton control
- **MoxAbletonTypes**: Log category (`LogMoxAbleton`) and connection state enum

#### Song Control
- `StartSong()` / `StopSong()` - Transport control via `/live/song/start_playing` and `/live/song/stop_playing`
- `StopAllClips()` - Emergency panic button via `/live/song/stop_all_clips`

#### Scene Control
- `FireScene(SceneID)` - Fire scene via `/live/scene/fire`

#### Clip Control
- `FireClip(TrackID, ClipSlot)` - Fire clip via `/live/clip/fire`
- `StopClip(TrackID, ClipSlot)` - Stop clip via `/live/clip/stop`

#### Track Control
- `StartTrack(TrackID)` / `StopTrack(TrackID)` - Track playback control
- `SetTrackVolume(TrackID, Volume)` - Volume control via `/live/track/set/volume`
- `SetTrackMute(TrackID, bMute)` - Mute control via `/live/track/set/mute`
- `SetTrackPanning(TrackID, Panning)` - Panning via `/live/track/set/panning`
- `SetTrackSolo(TrackID, bSolo)` - Solo via `/live/track/set/solo`

#### Device Control
- `SetDeviceParameter(TrackID, DeviceID, ParameterID, Value)` - Any device parameter via `/live/device/set/parameter/value`

#### Custom Events
- `SendCustomEvent(EventPath, Parameters)` - Send any AbletonOSC command
- **Automatic type detection**: Parameters auto-detected as int (`"42"`), float (`"0.75"`), or string (`"MyClip"`)

#### Component Properties
- `AbletonTrackID` - Track number (1-based, 0 = disabled)
- `AbletonClipSlot` - Clip slot number (1-based, 0 = disabled)
- `AbletonSceneID` - Scene number (1-based, 0 = disabled)
- Property-based functions: call `FireClip()` without parameters, uses configured IDs

#### Configuration
- INI-based configuration via `MozConfig.ini` section `[moxableton]`
- Configurable Ableton IP and port
- Auto-start option
- Granular logging flags (`logCommands`, `logOSCMessages`, `logCustomEvents`)

#### Safety Features
- **Automatic emergency stop**: `StopAllClips()` sent on subsystem shutdown
- **Parameter validation**: Component warns when required IDs are not configured
- **Auto registration**: Components register/unregister on BeginPlay/EndPlay

#### Documentation
- README.md with Quick Start, Blueprint & C++ guides
- DOCUMENTATION.md with full technical reference
- CHANGELOG.md

### Technical Details
- **Engine Version**: Unreal Engine 5.7
- **Protocol**: AbletonOSC via UDP
- **Send Port**: 11000 (configurable)
- **Blueprint Category**: `MozXR|Ableton`
- **Log Category**: `LogMoxAbleton`
- **Dependencies**: MoxUtility, OSC, Sockets, Networking
