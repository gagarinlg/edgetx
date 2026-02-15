# EdgeTX Architecture Documentation

## Table of Contents
- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Firmware Architecture](#firmware-architecture)
- [Companion Architecture](#companion-architecture)
- [Build System](#build-system)
- [Key Subsystems](#key-subsystems)

## Overview

EdgeTX is a sophisticated multi-platform radio control firmware system consisting of three main components:

1. **Radio Firmware** - Embedded firmware running on ARM-based radio transmitters
2. **Companion Application** - Desktop application for model management and simulation
3. **Simulator** - Radio emulation environment for testing and training

The project uses a dual-toolchain build system to support both embedded ARM targets and native desktop platforms (Linux, Windows, macOS).

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    EdgeTX Ecosystem                          │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────────┐      ┌───────────────────────────┐   │
│  │  Radio Firmware  │◄────►│  Companion Application    │   │
│  │  (ARM Embedded)  │ USB  │  (Qt Desktop)             │   │
│  │                  │      │  - Model Editor           │   │
│  │  - Control Logic │      │  - Firmware Flasher       │   │
│  │  - Audio Engine  │      │  - Backup/Restore         │   │
│  │  - Mixer System  │      │  - Simulator Interface    │   │
│  │  - UI Layers     │      └───────────────────────────┘   │
│  │  - Lua Runtime   │               │                       │
│  └──────────────────┘               │                       │
│         │                            ▼                       │
│         │                   ┌────────────────┐              │
│         │                   │  libsimulator  │              │
│         └──────────────────►│  (Firmware     │              │
│           Shared Code       │   Emulation)   │              │
│                             └────────────────┘              │
│                                                               │
│  ┌──────────────────────────────────────────────────────┐  │
│  │              SD Card Content                          │  │
│  │  - Lua Scripts    - Themes      - Sound Packs        │  │
│  │  - Model Files    - Bitmaps     - Translations       │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## Firmware Architecture

The radio firmware is organized into layers, from hardware abstraction to user interface:

### Layer Structure

```
┌─────────────────────────────────────────────────────────┐
│                  User Interface Layer                    │
│  - GUI Modules (128x64, 212x64, 480x272, 320x480)      │
│  - Menus, Views, Widgets, Themes                        │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│                 Application Layer                        │
│  - Model Manager    - Telemetry     - Trainer           │
│  - Lua Scripting    - Mixer Engine  - Curves            │
│  - Audio System     - Storage       - CLI               │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│              Hardware Abstraction Layer (HAL)            │
│  - Board Definitions                                     │
│  - Driver Interfaces (Keys, LCD, Audio, RF, Storage)    │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│                  Hardware Drivers                        │
│  - STM32 HAL (F2/F4/H7/H7RS)                           │
│  - Peripheral Drivers (SPI, I2C, UART, USB, DMA)       │
│  - Third-party Libraries (Lua, LZ4, FreeRTOS)          │
└─────────────────────────────────────────────────────────┘
```

### Key Directories in `radio/src/`

| Directory | Purpose |
|-----------|---------|
| `boards/` | Board-specific implementations and HAL |
| `targets/` | Target radio definitions (40+ models) |
| `gui/` | User interface implementations by screen type |
| `lua/` | Lua scripting engine and API |
| `storage/` | Model and settings storage (EEPROM/Flash) |
| `thirdparty/` | External dependencies (STM32 HAL, Lua, LZ4) |
| `bootloader/` | Bootloader for firmware updates |
| `bitmaps/` | UI graphics and icons |
| `fonts/` | Display fonts for various languages |

### Radio Target Structure

Each radio target is defined by:
- **Board Definition** (`boards/<board>/`) - Hardware configuration
- **Target Definition** (`targets/<target>/`) - Radio-specific settings
- **CMake Configuration** - Build flags and compiler options

Example radio families:
- **FrSky**: X9D+, X9E, X10, X12S, X-Lite
- **Jumper**: T12, T16, T18, T-Pro
- **Radiomaster**: TX12, TX16S, Zorro
- **Flysky**: NV14, PL18
- **TBS**: Tango 2

## Companion Architecture

The Companion application is a Qt-based desktop application written in C++.

### Component Structure

```
companion/src/
├── appdata.cpp/h           # Model data structures
├── eeprominterface.cpp/h   # Firmware interface abstraction
├── firmwareinterface.cpp/h # Firmware version handling
├── mainwindow.cpp/h        # Main application window
├── modeledit/              # Model editor UI
├── generaledit/            # Radio settings editor
├── simulation/             # Simulator interface
├── flashfirmwaredialog.*   # Firmware flashing
├── storage/                # File I/O and backup
└── translations/           # Internationalization
```

### Data Flow

```
User Action → Qt UI → Data Model → Firmware Interface → Radio/Simulator
                                        ↓
                                  File System (Models, Settings)
```

## Build System

EdgeTX uses a sophisticated CMake-based build system with a "superbuild" pattern.

### Build Architecture

```
CMakeLists.txt (Root)
    │
    ├─► ExternalProject: native
    │   └─► Toolchain: native.cmake
    │       ├─► Companion (Qt)
    │       ├─► Simulator (Qt + libsimulator)
    │       └─► Tests (GTest)
    │
    └─► ExternalProject: arm-none-eabi
        └─► Toolchain: arm-none-eabi.cmake
            ├─► Firmware (ARM embedded)
            └─► Bootloader (ARM embedded)
```

### Build Targets

| Target | Description | Toolchain |
|--------|-------------|-----------|
| `companion` | Desktop application | Native |
| `simulator` | Radio simulator GUI | Native |
| `libsimulator` | Firmware emulation library | Native |
| `gtests-radio` | Unit tests | Native |
| `firmware` | Radio firmware | ARM |
| `bootloader` | Radio bootloader | ARM |

### Version Management

Version is defined in root `CMakeLists.txt`:
- `VERSION_MAJOR.VERSION_MINOR.VERSION_REVISION` (e.g., 3.0.0)
- `CODENAME` for development branches
- Environment variables: `EDGETX_VERSION_TAG`, `EDGETX_VERSION_PREFIX`, `EDGETX_VERSION_SUFFIX`

## Key Subsystems

### 1. Mixer System

The heart of EdgeTX - processes inputs and generates outputs for servos/channels.

**Location**: `radio/src/mixer.cpp`, `radio/src/mixer_scheduler.cpp`

**Flow**:
```
Inputs (Sticks, Switches, Pots) → Mixer Lines → Curves → Limits → Outputs
                                       ↓
                                  Custom Functions
                                  Logical Switches
```

### 2. Audio System

Handles voice prompts, tones, and audio playback.

**Location**: `radio/src/audio.cpp`, `radio/src/audio_*_driver.cpp`

**Features**:
- Voice announcements
- Audio warnings/alarms
- Background music
- Multi-language support

### 3. Telemetry System

Processes and displays telemetry data from receivers.

**Location**: `radio/src/telemetry/` 

**Supported Protocols**:
- FrSky (SmartPort, D-series)
- Spektrum
- Flysky
- Multi-protocol
- Crossfire
- Ghost

### 4. Lua Scripting Engine

Embedded Lua interpreter for custom scripts.

**Location**: `radio/src/lua/`, `radio/src/thirdparty/Lua/`

**Script Types**:
- Model scripts (run continuously)
- Telemetry screens
- One-time functions
- Widgets (color LCD)

**API**: Documented at https://luadoc.edgetx.org/

### 5. Storage System

Manages persistent data on SD card and flash.

**Location**: `radio/src/storage/`

**Storage Types**:
- Model files (`.yml`, `.bin`)
- Radio settings (`.yml`, `.bin`)
- SD card content (Lua, themes, sounds)

### 6. GUI System

Multiple GUI implementations for different screen types.

**Locations**:
- `radio/src/gui/128x64/` - Monochrome 128x64 displays
- `radio/src/gui/212x64/` - Grayscale displays
- `radio/src/gui/colorlcd/` - Color LCD (480x272, 320x480)

**Components**:
- Views (Model, Radio, Telemetry)
- Widgets (Text, Buttons, Sliders)
- Layout engine
- Touch support (color LCD)
- Theme system

### 7. RF Protocols

External and internal RF module support.

**Location**: `radio/src/pulses/`

**Supported Protocols**:
- PPM
- DSM2/DSMX
- Multiprotocol
- Crossfire
- PXX (FrSky)
- AFHDS (Flysky)

### 8. Bootloader

Secure firmware update mechanism.

**Location**: `radio/src/bootloader/`

**Features**:
- USB DFU (Device Firmware Update)
- SD card update
- Firmware verification (CRC)
- Rollback protection

## Data Structures

### Model Data (`radio/src/datastructs*.h`)

Key structures:
- `ModelData` - Complete model configuration
- `MixData` - Mixer line definition
- `ExpoData` - Input curve/expo settings
- `LogicalSwitchData` - Logical switch configuration
- `CustomFunctionData` - Special function settings

### Radio Settings (`radio/src/dataconstants.h`)

- General settings
- Trainer configuration
- Module settings
- Calibration data

## Threading Model

### Firmware (FreeRTOS)
- Main mixer task (highest priority)
- Audio task
- Telemetry processing
- GUI rendering
- Storage operations

### Companion (Qt)
- Main UI thread
- Worker threads for:
  - Firmware download
  - File operations
  - Simulator communication

## Inter-Process Communication

### Radio ↔ Companion
- **USB Serial**: Model transfer, settings sync
- **USB DFU**: Firmware flashing
- **USB Storage**: SD card access (some radios)

### Companion ↔ Simulator
- Shared memory (libsimulator)
- Direct function calls to emulated firmware

## Development Workflow

```
1. Checkout code → 2. Configure CMake → 3. Build target
                         ↓
4. Test (simulator/hardware) → 5. Debug → 6. Commit
                                   ↓
7. CI/CD (GitHub Actions) → 8. Release (GitHub Releases)
```

## Further Reading

- [BUILD.md](BUILD.md) - Detailed build instructions
- [CONTRIBUTING.md](CONTRIBUTING.md) - Development guidelines
- [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) - Code structure and debugging
- [HARDWARE_SUPPORT.md](HARDWARE_SUPPORT.md) - Adding new radio support
- [EdgeTX Wiki](https://github.com/EdgeTX/edgetx/wiki) - Additional resources
