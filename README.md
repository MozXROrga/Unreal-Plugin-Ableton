# MoxAbleton Plugin

**Status:** Stable · **Suite version:** 3.0.0 · **Unreal Engine:** 5.7

MoxAbleton connects Unreal workflows with Ableton Live 11 through the project’s AbletonOSC setup. It supports sends, listeners, and bidirectional events.

## Included examples

Four active Ableton Live 11 projects are included under MozXR/Ableton: Listen, Rotation, Scene, and Song. Correspondingly named Unreal example-map families are included. Ableton Backup folders are recovery material, not Quickstart projects.

## Activation

Enable MoxAbleton and OSC support, configure endpoints through project configuration, then open a matching Live project and Unreal example map.

See [Quickstart](QUICKSTART.md), [C++ documentation](DOCUMENTATION.md), and [changelog](CHANGELOG.md).

Real-time bridge for Unreal Engine 5 and Ableton Live. Controls clips, scenes, tracks, and song playback via **AbletonOSC** protocol.

---

## Features

✅ **AbletonOSC Protocol**: Full control over Ableton Live via OSC  
✅ **Clip Control**: Fire/stop clips by track + slot  
✅ **Scene Control**: Fire/stop scenes  
✅ **Track Control**: Play/stop, volume, mute, panning, solo  
✅ **Song Transport**: Start/stop playback  
✅ **Device Control**: Set any device parameter  
✅ **Custom Events**: Send any AbletonOSC command  
✅ **Emergency Stop**: Panic button for all clips  
✅ **Blueprint & C++ API**: Full support for both workflows  
✅ **Component-Based**: Attach to any actor for per-object Ableton control  
✅ **INI Configuration**: Project-wide settings via MozConfig.ini  
✅ **Auto Type Detection**: Custom event parameters auto-detected as int/float/string  
✅ **Automatic Shutdown**: Emergency stop sent on application exit  

---

## Requirements

| Requirement | Details |
|-------------|---------|
| **Unreal Engine** | 5.7+ |
| **Ableton Live** | 10+ (with AbletonOSC installed) |
| **AbletonOSC** | [github.com/ideoforms/AbletonOSC](https://github.com/ideoforms/AbletonOSC) |

### AbletonOSC Setup

1. Download AbletonOSC from GitHub
2. Copy to Ableton Live's Remote Scripts folder
3. In Ableton: Preferences → Link/Tempo/MIDI → Control Surface → **AbletonOSC**
4. AbletonOSC listens on port **11000** and responds on port **11001**

---

## Quick Start (5 Minutes)

### 1. Enable Plugin

Edit → Plugins → Search **"MoxAbleton"** → Enable → Restart Editor

### 2. Configure Connection

Edit `Project/Config/MozConfig.ini`:

```ini
[moxableton]
enabled=true
abletonIP=<configured-address>
abletonPort=11000
```

### 3. Add Component to Actor

1. Open your Actor Blueprint
2. Add Component → Search **"Mox Ableton Component"**
3. Set **AbletonTrackID** (1-based, matching Ableton's display)
4. Set **AbletonClipSlot** (1-based, matching Ableton's display)
5. Call **Fire Clip** from an event

### 4. Test

1. Start AbletonOSC in Ableton Live
2. Play in Editor
3. Trigger the event → Clip should fire in Ableton 🎹

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      MoxAbleton Plugin                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌──────────────────────┐     ┌──────────────────────┐                      │
│  │  MoxAbletonComponent │────▶│  MoxAbletonSubsystem │                      │
│  │  (per actor)         │     │  (GameInstance)      │                      │
│  └──────────────────────┘     └──────────┬───────────┘                      │
│                                          │                                  │
│                                          ▼                                  │
│                               ┌──────────────────────┐                      │
│                               │  UDP Socket          │                      │
│                               │  → AbletonOSC        │                      │
│                               │  (Port 11000)        │                      │
│                               └──────────────────────┘                      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Dependencies

| Module | Purpose |
|--------|---------|
| **OSC** | Unreal Engine's official OSC plugin |
| **Core, CoreUObject, Engine** | Unreal Engine fundamentals |
| **Networking, Sockets** | UDP socket communication |

---

## Configuration

### MozConfig.ini

Edit `Project/Config/MozConfig.ini`:

```ini
[moxableton]
#==============================================================================
# Network Configuration
#==============================================================================
abletonIP=<configured-address>        # Ableton Live machine IP
abletonPort=11000           # AbletonOSC listen port (default: 11000)

#==============================================================================
# System Control
#==============================================================================
enabled=true                # Auto-start on game begin

#==============================================================================
# Logging
#==============================================================================
logCommands=true            # Log clip/scene/track/song commands
logOSCMessages=false        # Log raw OSC packet details (verbose!)
logCustomEvents=true        # Log custom event invocations
```

---

## Blueprint Usage

### 1. Component Properties

| Property | Type | Description |
|----------|------|-------------|
| **AbletonTrackID** | int32 | Track number (1-based as in Ableton, 0 = disabled) |
| **AbletonClipSlot** | int32 | Clip slot number (1-based as in Ableton, 0 = disabled) |
| **AbletonSceneID** | int32 | Scene number (1-based as in Ableton, 0 = disabled) |

### 2. Component Functions

**Clip Control** (`MozXR|Ableton|Clip`):
- `FireClip()` – Fire clip at configured TrackID + ClipSlot
- `StopClip()` – Stop clip at configured TrackID + ClipSlot

**Scene Control** (`MozXR|Ableton|Scene`):
- `FireScene()` – Fire scene at configured SceneID

**Track Control** (`MozXR|Ableton|Track`):
- `StopTrack()` – Stop all clips on configured TrackID
- `SetTrackVolume(float)` – Set volume (0.0-1.0)
- `SetTrackMute(bool)` – Mute/unmute

**Song Control** (`MozXR|Ableton|Song`):
- `StartSong()` – Start playback
- `StopSong()` – Stop playback

**Advanced** (`MozXR|Ableton|Advanced`):
- `SendCustomEvent(EventPath, Parameters)` – Send any AbletonOSC command

### 3. Subsystem Functions (Globa…60230 tokens truncated…ngine**: 5.7
- **Platform**: Windows 11 64-bit
- **Visual Studio**: 2022 (for C++ development)

### Network
- **Protocol**: UDP Multicast or Unicast
- **Bandwidth**: ~1-5 Mbps per tracker
- **Latency**: <10ms recommended

---

## Performance

### Expected Metrics

| Metric | Target | Typical |
|--------|--------|---------|
| Update Rate | 60-120 FPS | 60 FPS |
| Latency | <16ms | ~8ms |
| Frame Impact | <1ms/100 tracks | ~0.5ms |
| Memory/Instance | <10MB | ~6MB |
| Network Thread CPU | <10% | ~5% |

### Optimization Tips

1. **Pre-allocate Actor Pool** for predictable performance
2. **Disable Verbose Logging** in production  
3. **Use Slot Mapping** to limit LiveLink subjects
4. **Limit LiveLink Slots** (`MaxSlots`) based on actual usage

---

## Support

**Created by:** Ars Electronica Futurelab - Johannes Lugstein | Friedrich Bachinger | Otto Naderer  
**Website**: https://ars.electronica.art/futurelab/  
**Copyright**: © 2020-2025 Ars Electronica Futurelab

For technical support, see [DOCUMENTATION.md](DOCUMENTATION.md) or contact the development team.

---

## Changelog

### v2.7.2 (2026-02-04)
- 🔧 **FIXED:** Floor-Wall Alignment with FloorRotation=90° - Wall perimeter order and WorldPositions corrected
  - Wall TUIO X=0 now correctly maps to RIGHT-FRONT corner (matches Floor's Y=0 front area)
  - Wall perimeter order changed: `RIGHT → BACK → LEFT → FRONT`
  - All Wall.WorldPosition.X values shifted by -1350 to align with Floor's negative X direction
- 📖 RootOriginActor now consistently defines (0,0,0) for both Floor and Wall tracking

### v2.7.0 (2026-02-04)
- ✨ **NEW:** WallRotation Configuration - Global wall rotation for coordinate alignment
- 🔧 **FIXED:** Wall Transformation Order - Scale now applied before Rotation (consistent with Floor)

### v2.6.0 (2026-02-03)
- 🔧 **FIXED:** Normalize Coordinates Bug - InvertY now correctly uses `1.0 - Y` for normalized coordinates
- 📚 **CHANGED:** Reference Actor Origin Position - Origin is now at top-left (TUIO standard) instead of bottom-left
- 📚 **NEW:** Comprehensive coordinate system documentation with TUIO origin explanation

### v2.3.2 (2026-01-14)
- 🔧 **CRITICAL FIX:** Fixed fatal crash "Cannot generate unique name" when `UseActorPool=false`
  - Tracks re-entering bounds now properly reuse existing actors instead of spawning duplicates
  - Actors are now properly hidden (instead of left visible) when `bAutoDestroyOnTrackLost=false`
- ✨ **NEW:** `FindExistingActorByName()` helper for actor reuse in Dynamic Spawn Mode

### v2.3.1 (2026-01-14)
- 🔧 **FIXED:** RootOriginActor Relative Orientation - Actors are now correctly oriented relative to the RootOriginActor

### v2.2.1 (2026-01-12)
- ✨ **NEW:** `bIsInsideBoundary` field in `FPharusTrackData` - indicates if track is inside valid bounds
- ✨ **NEW:** Extended `GetTrackData()` with `bOutIsInsideBoundary` output parameter
- 📚 Track boundary status now available in Blueprints for visibility control

### v2.2.0 (2026-01-12)
- ✨ **NEW:** Root Origin Rotation Support - `AMoxPharusRootOriginActor` rotation now applied to all tracking
- ✨ **NEW:** `GetRootOriginRotation()` / `SetRootOriginRotation()` Blueprint methods
- ✨ **NEW:** `GlobalRotation` config parameter for static rotation
- 🔧 **FIXED:** Rotation bug where actors always stayed horizontally aligned

### v2.1.0 (2025-12-10)
- ✨ **NEW:** Corner-based Wall Mapping - `WorldPosition` now defines anchor point (bottom-left corner)
- ✨ **NEW:** Seamless wall transitions with correct Yaw rotations
- ✨ **NEW:** `Origin` parameter for coordinate system offset
- 🔧 **FIXED:** Wall mapping offset issue where (0,0) appeared in wall center instead of corner
- 🔧 **FIXED:** InvertY now correctly flips Y-axis (1-Y instead of -Y)
- 📚 Updated documentation with wall mapping diagrams and Yaw reference table

### v2.0.0 (2025-11-25)
- ✨ **NEW:** `SetSpawnClassOverride()` on Subsystem - Override DefaultSpawnClass before StartPharusSystem()
- ✨ **NEW:** `SetSpawnClass()` / `GetSpawnClass()` on Instance - Change SpawnClass at runtime
  - Allows setting custom actor classes per instance from Blueprint/C++
  - New tracks will use the new class, existing tracks are not affected

### v1.0.0 (2025-10-31)
- ✨ **NEW:** Manual Start/Stop control via Blueprint functions
  - `StartPharusSystem()` - Start tracking manually
  - `StopPharusSystem()` - Stop all tracking
  - `IsPharusSystemRunning()` - Check system status
- ✨ **NEW:** `AutoStartSystem` config option for manual control
- ✨ **FIXED:** `EnableLiveLink` flag now correctly controls actor spawning
  - `true` = Actors + LiveLink subjects
  - `false` = LiveLink-only mode (no actors)
- 📚 Updated documentation with new features
- ✨ GameInstance Subsystem architecture
- ✨ Multi-instance support (Floor + Wall)
- ✨ LiveLink integration with slot mapping
- ✨ nDisplay actor pool for cluster synchronization
- ✨ Flexible mapping modes (Simple, Regions)
- ✨ Live configuration adjustments
- ✨ Debug logging control
- ✨ Thread-safe design with smart pointers
- 🔧 Performance optimizations
- 📚 Comprehensive documentation

---

**Last Updated**: 2026-02-04
**Plugin Version**: 2.8.0
**Minimum UE Version**: 5.7

