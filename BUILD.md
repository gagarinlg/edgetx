# EdgeTX Build Guide

Complete guide for building EdgeTX firmware, companion, and simulator from source.

## Table of Contents
- [Quick Start](#quick-start)
- [System Requirements](#system-requirements)
- [Installing Dependencies](#installing-dependencies)
- [Building Firmware](#building-firmware)
- [Building Companion](#building-companion)
- [Building Simulator](#building-simulator)
- [Build Options](#build-options)
- [Troubleshooting](#troubleshooting)

## Quick Start

### Using Docker (Recommended for Beginners)

```bash
# 1. Clone repository
git clone --recursive https://github.com/EdgeTX/edgetx.git
cd edgetx

# 2. Build using Docker
docker pull ghcr.io/edgetx/build-edgetx:main
docker run --rm -v $(pwd):/src ghcr.io/edgetx/build-edgetx:main

# Firmware will be in build-output/firmware/
```

### Manual Build (Linux)

```bash
# 1. Install dependencies
sudo apt-get install cmake gcc-arm-none-eabi python3

# 2. Clone and build
git clone --recursive https://github.com/EdgeTX/edgetx.git
cd edgetx
cmake --preset default
cmake --build build-default --target firmware-X10
```

## System Requirements

### Minimum Requirements

- **CPU**: Dual-core processor (quad-core recommended for parallel builds)
- **RAM**: 4 GB (8 GB recommended)
- **Disk Space**: 5 GB for source + build artifacts
- **OS**: Linux, Windows 10+, macOS 10.14+

### Software Requirements

**Essential:**
- CMake 3.13 or later
- Git
- Python 3.7 or later

**For Firmware:**
- ARM GCC Toolchain (arm-none-eabi-gcc) 10.3 or later

**For Companion/Simulator:**
- Qt 5.15 or later
- C++ compiler (GCC 7+, Clang 8+, MSVC 2017+)

## Installing Dependencies

### Linux (Ubuntu/Debian)

```bash
# Essential tools
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  git \
  python3 \
  python3-pip \
  python3-clang

# ARM toolchain for firmware
sudo apt-get install -y gcc-arm-none-eabi

# Qt for companion/simulator
sudo apt-get install -y \
  qtbase5-dev \
  qtmultimedia5-dev \
  qttools5-dev \
  libqt5svg5-dev \
  libqt5opengl5-dev

# Optional: SDL2 for simulator joystick support
sudo apt-get install -y libsdl2-dev

# Optional: Code formatting
sudo apt-get install -y clang-format
```

### Linux (Fedora/RHEL)

```bash
# Essential tools
sudo dnf install -y \
  cmake \
  git \
  python3 \
  gcc \
  gcc-c++

# ARM toolchain
sudo dnf install -y arm-none-eabi-gcc-cs

# Qt for companion
sudo dnf install -y \
  qt5-qtbase-devel \
  qt5-qtmultimedia-devel \
  qt5-qttools-devel \
  qt5-qtsvg-devel
```

### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake git python3 qt@5

# Install ARM toolchain
brew tap osx-cross/arm
brew install arm-gcc-bin

# Add to PATH
export PATH="/usr/local/opt/qt@5/bin:$PATH"
```

### Windows

**Option 1: Using MSYS2 (Recommended)**

```bash
# Download and install MSYS2 from https://www.msys2.org/

# Open MSYS2 MinGW 64-bit terminal
pacman -Syu
pacman -S --needed base-devel mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-qt5
pacman -S mingw-w64-x86_64-arm-none-eabi-gcc
pacman -S git python3
```

**Option 2: Manual Installation**

1. **CMake**: Download from https://cmake.org/download/
2. **Qt**: Download from https://www.qt.io/download
3. **ARM Toolchain**: Download from https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
4. **Python**: Download from https://www.python.org/downloads/
5. **Git**: Download from https://git-scm.com/download/win

Add all tools to your PATH.

## Building Firmware

### Step 1: Clone Repository

```bash
git clone --recursive https://github.com/EdgeTX/edgetx.git
cd edgetx
```

If you forgot `--recursive`:
```bash
git submodule update --init --recursive
```

### Step 2: Configure Build

EdgeTX uses CMake presets for common configurations:

```bash
# List available presets
cmake --list-presets

# Use default preset (builds all targets)
cmake --preset default
```

### Step 3: Build Firmware

**Build specific radio target:**

```bash
# Syntax: firmware-<TARGET_NAME>
cmake --build build-default --target firmware-X10
cmake --build build-default --target firmware-X12S
cmake --build build-default --target firmware-TX16S
cmake --build build-default --target firmware-ZORRO
```

**Build all firmware targets:**

```bash
cmake --build build-default --target firmware
```

**Available targets** (see `fw.json` for complete list):
- FrSky: `X9D`, `X9DP`, `X9E`, `X10`, `X10E`, `X12S`, `XLITE`, `XLITES`, `X9LITE`
- Jumper: `T12`, `T16`, `T18`, `TLITE`, `TPRO`
- Radiomaster: `TX12`, `TX12MK2`, `TX16S`, `ZORRO`
- Flysky: `NV14`, `PL18`
- TBS: `TANGO2`

### Step 4: Locate Built Firmware

Firmware files are in:
```
build-default/arm-none-eabi/firmware/
├── firmware-X10.bin     (firmware file)
└── firmware-X10.elf     (debug symbols)
```

### Step 5: Flash Firmware

**Method 1: Using Companion**
1. Open EdgeTX Companion
2. Go to "Download" → "Download Firmware"
3. Select "Use custom firmware file"
4. Choose the `.bin` file
5. Flash to radio via USB

**Method 2: Bootloader (Radio)**
1. Copy `.bin` file to SD card root as `FIRMWARE.BIN`
2. Power on radio while holding trim switches
3. Select "Write Firmware"

**Method 3: DFU (USB)**
```bash
# Install dfu-util
sudo apt-get install dfu-util

# Flash firmware
dfu-util -a 0 -D build-default/arm-none-eabi/firmware/firmware-X10.bin
```

## Building Companion

### Configure and Build

```bash
# Configure with default preset
cmake --preset default

# Build companion
cmake --build build-default --target companion

# Run companion
./build-default/native/companion
```

### Platform-Specific Notes

**Linux:**
- Ensure Qt5 is in your PATH
- Run with: `./build-default/native/companion`

**macOS:**
- Use Qt5 from Homebrew
- May need to set Qt5_DIR: `export Qt5_DIR=/usr/local/opt/qt@5/lib/cmake/Qt5`
- Run with: `open build-default/native/companion.app`

**Windows:**
- Ensure Qt5 DLLs are accessible (add to PATH or copy to build dir)
- Run with: `build-default\native\companion.exe`

### Creating Installer Package

```bash
# Linux: Create AppImage
cmake --build build-default --target package

# Windows: Create installer
cmake --build build-default --target package

# macOS: Create DMG
cmake --build build-default --target package
```

## Building Simulator

The simulator allows testing radio firmware on desktop without hardware.

### Build Simulator

```bash
# Build simulator library and GUI
cmake --build build-default --target simulator

# Run simulator
./build-default/native/simulator
```

### Simulator Features

- Full radio firmware emulation
- Joystick support (via SDL2)
- Model import/export
- Telemetry simulation
- Debug tools

### Radio Simulator Targets

Build simulator for specific radio:

```bash
# Syntax: simu-<TARGET_NAME>
cmake --build build-default --target simu-X10
cmake --build build-default --target simu-TX16S
```

## Build Options

### CMake Configuration Options

Configure options with `-D` flag:

```bash
cmake --preset default -DOPTION=VALUE
```

### Common Options

| Option | Values | Description |
|--------|--------|-------------|
| `CMAKE_BUILD_TYPE` | `Debug`, `Release` | Build type |
| `PCB` | Target name | Radio target (X10, TX16S, etc.) |
| `DEFAULT_MODE` | 1-4 | Default stick mode |
| `GVARS` | `YES`, `NO` | Global variables support |
| `LUA` | `YES`, `NO` | Lua scripting |
| `HELI` | `YES`, `NO` | Helicopter functions |
| `FLIGHT_MODES` | `YES`, `NO` | Flight modes support |
| `CURVES` | `YES`, `NO` | Custom curves |
| `MULTIMODULE` | `YES`, `NO` | Multi-protocol module |
| `AFHDS2` | `YES`, `NO` | AFHDS2 protocol |
| `AFHDS3` | `YES`, `NO` | AFHDS3 protocol |
| `CROSSFIRE` | `YES`, `NO` | TBS Crossfire |
| `GHOST` | `YES`, `NO` | ImmersionRC Ghost |
| `PXX1` | `YES`, `NO` | FrSky PXX protocol |
| `PXX2` | `YES`, `NO` | FrSky ACCESS protocol |
| `DEBUG` | `YES`, `NO` | Debug build |
| `TRACE_SD_CARD` | `YES`, `NO` | SD card debug traces |
| `TRACE_SIMPGMSPACE` | `YES`, `NO` | PROGMEM debug traces |
| `DISK_CACHE` | `YES`, `NO` | File system caching |

### Build Type Examples

**Minimal firmware (for testing):**
```bash
cmake --preset default \
  -DPCB=X10 \
  -DLUA=NO \
  -DHELI=NO \
  -DFLIGHT_MODES=NO \
  -DCURVES=NO
cmake --build build-default --target firmware-X10
```

**Debug build with traces:**
```bash
cmake --preset default \
  -DPCB=X10 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DDEBUG=YES \
  -DTRACE_SD_CARD=YES
cmake --build build-default --target firmware-X10
```

**Full-featured build:**
```bash
cmake --preset default \
  -DPCB=TX16S \
  -DLUA=YES \
  -DHELI=YES \
  -DFLIGHT_MODES=YES \
  -DMULTIMODULE=YES \
  -DCROSSFIRE=YES \
  -DGHOST=YES
cmake --build build-default --target firmware-TX16S
```

## Advanced Build Topics

### Parallel Builds

Speed up compilation:

```bash
# Use all CPU cores
cmake --build build-default --parallel

# Use specific number of cores
cmake --build build-default --parallel 4
```

### Custom Toolchain

Use different ARM toolchain:

```bash
cmake --preset default \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/arm-none-eabi.cmake \
  -DARM_TOOLCHAIN_PATH=/path/to/toolchain
```

### Out-of-Source Builds

Keep source tree clean:

```bash
mkdir build-custom
cd build-custom
cmake .. -DCMAKE_BUILD_TYPE=Release -DPCB=X10
cmake --build . --target firmware-X10
```

### Verbose Build Output

See full compiler commands:

```bash
cmake --build build-default --target firmware-X10 --verbose
```

### Clean Builds

```bash
# Clean build artifacts
cmake --build build-default --target clean

# Complete clean (remove build directory)
rm -rf build-default
cmake --preset default
```

## Build System Architecture

### Superbuild Pattern

EdgeTX uses CMake's ExternalProject for a "superbuild":

```
Root CMakeLists.txt
  ├─► Native Build (x86/x64)
  │   ├─► Companion (Qt)
  │   ├─► Simulator
  │   └─► Tests
  └─► ARM Build (arm-none-eabi)
      ├─► Firmware
      └─► Bootloader
```

### Build Directory Structure

```
edgetx/
├── build-default/
│   ├── native/              # Desktop builds
│   │   ├── companion        # Companion app
│   │   ├── simulator        # Simulator app
│   │   └── gtests-radio     # Unit tests
│   └── arm-none-eabi/       # Firmware builds
│       ├── firmware/        # Firmware .bin files
│       └── bootloader/      # Bootloader files
```

### Target Organization

Build targets follow pattern: `<type>-<target>`

- `firmware-X10` - Firmware for X10
- `bootloader-X10` - Bootloader for X10
- `simu-X10` - Simulator for X10

## Continuous Integration

EdgeTX uses GitHub Actions for CI:

- `.github/workflows/build_fw.yml` - Firmware builds
- `.github/workflows/build_companion.yml` - Companion builds
- Tests run on every commit
- Artifacts available for download

## Troubleshooting

### Common Issues

**Problem: `arm-none-eabi-gcc: command not found`**

Solution:
```bash
# Linux
sudo apt-get install gcc-arm-none-eabi

# macOS
brew install arm-gcc-bin

# Verify installation
arm-none-eabi-gcc --version
```

**Problem: CMake can't find Qt**

Solution:
```bash
# Set Qt path explicitly
export Qt5_DIR=/path/to/qt5/lib/cmake/Qt5
# or
cmake --preset default -DQt5_DIR=/path/to/qt5/lib/cmake/Qt5
```

**Problem: Git submodules not initialized**

Solution:
```bash
git submodule update --init --recursive
```

**Problem: Build fails with "undefined reference"**

Solution:
- Clean build: `rm -rf build-default && cmake --preset default`
- Update submodules: `git submodule update --recursive`
- Check toolchain version

**Problem: Firmware too large for flash**

Solution:
- Build Release instead of Debug: `-DCMAKE_BUILD_TYPE=Release`
- Disable unused features: `-DLUA=NO`, `-DHELI=NO`, etc.
- Check with: `cmake --build build-default --target firmware-X10-size`

**Problem: Python scripts fail**

Solution:
```bash
# Install required Python packages
pip3 install Pillow PyYAML
```

**Problem: Windows: "The system cannot find the path specified"**

Solution:
- Use forward slashes in paths
- Keep path length under 260 characters
- Run from MSYS2 MinGW terminal, not CMD

### Getting Help

1. Check [GitHub Issues](https://github.com/EdgeTX/edgetx/issues)
2. Search [GitHub Discussions](https://github.com/EdgeTX/edgetx/discussions)
3. Ask on [Discord](https://discord.gg/wF9wUKnZ6H)
4. Read [Development Wiki](https://github.com/EdgeTX/edgetx/wiki)

## Building for Distribution

### Firmware Release Build

```bash
# Set version tag
export EDGETX_VERSION_TAG="v2.10.0"

# Build all targets
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build-default --target firmware --parallel

# Firmware files in build-default/arm-none-eabi/firmware/
```

### Companion Release Build

```bash
# Configure with version
cmake --preset default -DCMAKE_BUILD_TYPE=Release

# Build and package
cmake --build build-default --target companion
cmake --build build-default --target package

# Installer in build-default/native/
```

## Next Steps

- Read [ARCHITECTURE.md](ARCHITECTURE.md) to understand code structure
- See [CONTRIBUTING.md](CONTRIBUTING.md) for development workflow
- Check [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) for debugging tips
- Explore [HARDWARE_SUPPORT.md](HARDWARE_SUPPORT.md) for adding new radios

## Additional Resources

- **Docker Build Environment**: https://github.com/EdgeTX/build-edgetx
- **Development Wiki**: https://github.com/EdgeTX/edgetx/wiki
- **Build Configurations**: See `fw.json` for target definitions
- **CI/CD Workflows**: See `.github/workflows/` for automation

Happy building! 🚀
