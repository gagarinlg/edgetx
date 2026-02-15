# EdgeTX Project Overview

This document provides a high-level overview of the EdgeTX project structure and documentation.

## What is EdgeTX?

EdgeTX is an open-source firmware for RC (radio control) transmitters. It's a community-driven project that provides cutting-edge features for controlling RC aircraft, drones, cars, boats, and other RC vehicles.

**Key Features:**
- Advanced mixer system with unlimited mixes
- Lua scripting support for custom functions
- Multi-protocol RF module support
- Telemetry from various protocols
- Color and monochrome display support
- Qt-based companion application for model management
- Cross-platform desktop simulator

## Project Components

### 1. Radio Firmware
**Location:** `radio/src/`

The embedded firmware that runs on the physical radio transmitter hardware. Written primarily in C/C++, it includes:
- Mixer engine for processing inputs and generating outputs
- Audio system for voice prompts and alerts
- GUI system supporting multiple screen types (monochrome, color)
- Lua scripting engine
- Telemetry processing
- Storage management (models, settings)
- RF protocol implementations

**Supported Platforms:**
- STM32F2/F4/F7/H7/H7RS microcontrollers
- 40+ different radio models from various manufacturers

### 2. Companion Application
**Location:** `companion/src/`

Desktop application (Qt-based) for:
- Model editing and management
- Radio settings configuration
- Firmware flashing
- Backup and restore
- Model simulation

**Platforms:** Windows, Linux, macOS

### 3. Simulator
**Location:** Integrated with companion

Radio firmware emulation for testing without hardware:
- Full firmware emulation
- Joystick support
- Model testing and training

## Documentation Structure

The project includes comprehensive documentation for various audiences:

### For Users
- **[README.md](README.md)** - Quick start and links
- **[User Manual](https://manual.edgetx.org/)** - End-user documentation (external)
- **[Lua Documentation](https://luadoc.edgetx.org/)** - Lua scripting reference (external)

### For Developers
- **[ARCHITECTURE.md](ARCHITECTURE.md)** (13 KB) - System architecture and design
  - Firmware and companion architecture
  - Build system structure
  - Key subsystems (mixer, audio, telemetry, GUI)
  - Data structures and threading model

- **[BUILD.md](BUILD.md)** (14 KB) - Build instructions
  - System requirements
  - Installing dependencies (Linux, macOS, Windows)
  - Building firmware, companion, simulator
  - Build options and configurations
  - Troubleshooting

- **[CONTRIBUTING.md](CONTRIBUTING.md)** (13 KB) - Contribution guidelines
  - Getting started
  - Development workflow
  - Coding standards and style
  - Pull request process
  - Testing guidelines
  - Community guidelines

- **[DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)** (16 KB) - Developer workflows
  - Code structure overview
  - IDE setup (VSCode, CLion, Qt Creator)
  - Debugging techniques (simulator, hardware)
  - Testing framework
  - Common development tasks
  - Performance optimization
  - Best practices

- **[API_REFERENCE.md](API_REFERENCE.md)** (16 KB) - API documentation
  - Mixer API
  - Audio API
  - Storage API
  - Telemetry API
  - GUI API
  - Lua API
  - HAL (Hardware Abstraction Layer) API

- **[HARDWARE_SUPPORT.md](HARDWARE_SUPPORT.md)** (16 KB) - Hardware porting guide
  - Understanding target structure
  - Board definition creation
  - Peripheral driver implementation
  - Build system integration
  - Testing procedures
  - Troubleshooting hardware issues

## Technology Stack

### Firmware
- **Language:** C/C++ (C++11)
- **RTOS:** FreeRTOS
- **MCU:** ARM Cortex-M series (STM32)
- **Build System:** CMake
- **Scripting:** Lua 5.4
- **Graphics:** Custom GUI frameworks for different displays

### Companion
- **Language:** C++ (C++11)
- **Framework:** Qt 5.15+
- **Build System:** CMake
- **Testing:** Qt Test framework

### Build Environment
- **CMake:** 3.13+
- **ARM Toolchain:** arm-none-eabi-gcc 10.3+
- **Docker:** For consistent builds
- **CI/CD:** GitHub Actions

## Directory Structure

```
edgetx/
├── radio/                      # Radio firmware
│   ├── src/                   # Firmware source code
│   │   ├── boards/           # Board-specific HAL
│   │   ├── targets/          # Radio target definitions
│   │   ├── gui/              # GUI implementations
│   │   ├── lua/              # Lua scripting
│   │   ├── telemetry/        # Telemetry protocols
│   │   ├── storage/          # Storage system
│   │   └── thirdparty/       # External libraries
│   ├── data/                 # Radio data files
│   └── util/                 # Build utilities
│
├── companion/                 # Desktop companion app
│   ├── src/                  # Companion source code
│   ├── targets/              # Build targets
│   └── util/                 # Utilities
│
├── cmake/                     # CMake build scripts
├── tools/                     # Build tools and scripts
├── .github/                   # GitHub workflows
│
├── docs/                      # Documentation directory
│   └── README.md             # Documentation index
│
├── ARCHITECTURE.md            # System architecture
├── BUILD.md                   # Build instructions
├── CONTRIBUTING.md            # Contribution guide
├── DEVELOPER_GUIDE.md         # Developer workflows
├── API_REFERENCE.md           # API documentation
├── HARDWARE_SUPPORT.md        # Hardware porting guide
├── README.md                  # Project overview
├── LICENSE                    # GPL v2 license
├── CMakeLists.txt            # Root CMake configuration
└── fw.json                   # Firmware target definitions
```

## Getting Started

### As a User
1. Download firmware from [GitHub Releases](https://github.com/EdgeTX/edgetx/releases)
2. Flash to your radio using [EdgeTX Companion](https://github.com/EdgeTX/edgetx/releases)
3. Configure using the radio or companion app
4. Refer to [User Manual](https://manual.edgetx.org/)

### As a Developer
1. **Understand the system:** Read [ARCHITECTURE.md](ARCHITECTURE.md)
2. **Set up environment:** Follow [BUILD.md](BUILD.md)
3. **Learn the workflow:** Read [CONTRIBUTING.md](CONTRIBUTING.md)
4. **Start coding:** Use [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) and [API_REFERENCE.md](API_REFERENCE.md)

### As a Hardware Porter
1. **Understand architecture:** Read [ARCHITECTURE.md](ARCHITECTURE.md)
2. **Study existing ports:** Review `radio/src/boards/` and `radio/src/targets/`
3. **Follow porting guide:** [HARDWARE_SUPPORT.md](HARDWARE_SUPPORT.md)
4. **Test thoroughly:** Use debugging techniques from [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)

## Development Process

### Workflow
1. **Find/Create Issue:** Discuss feature or bug on GitHub
2. **Fork & Branch:** Create feature branch
3. **Develop:** Make changes, test in simulator
4. **Test:** Run unit tests and hardware tests
5. **Submit PR:** Follow PR template and guidelines
6. **Code Review:** Address feedback
7. **Merge:** Maintainer merges when approved

### Code Standards
- **Style:** Enforced by `.clang-format`
- **Commits:** Follow [Conventional Commits](https://conventionalcommits.org/)
- **Testing:** Unit tests with GoogleTest
- **Documentation:** Update docs with code changes

## Community

### Communication Channels
- **Discord:** https://discord.gg/wF9wUKnZ6H (Primary chat)
- **GitHub Discussions:** For long-form discussions
- **GitHub Issues:** For bugs and feature requests
- **Facebook Group:** Community discussions

### Contributing
Contributions are welcome! Whether you're:
- Fixing a bug
- Adding a feature
- Improving documentation
- Porting to new hardware
- Helping other users

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## Project Statistics

- **Lines of Code:** ~500,000+ (firmware + companion)
- **Source Files:** ~2,700+ C/C++ files
- **Supported Radios:** 40+ models
- **Contributors:** 100+ active contributors
- **License:** GPL v2

## Documentation Summary

| Document | Size | Purpose | Audience |
|----------|------|---------|----------|
| README.md | 4 KB | Project overview | All |
| ARCHITECTURE.md | 13 KB | System design | Developers |
| BUILD.md | 14 KB | Build instructions | Developers |
| CONTRIBUTING.md | 13 KB | Contribution guide | Contributors |
| DEVELOPER_GUIDE.md | 16 KB | Development workflows | Developers |
| API_REFERENCE.md | 16 KB | API documentation | Developers |
| HARDWARE_SUPPORT.md | 16 KB | Hardware porting | Hardware devs |

**Total Documentation:** ~90 KB of comprehensive developer documentation

## External Resources

- **Website:** https://edgetx.org/
- **User Manual:** https://manual.edgetx.org/
- **Lua Documentation:** https://luadoc.edgetx.org/
- **Development Wiki:** https://github.com/EdgeTX/edgetx/wiki
- **Releases:** https://github.com/EdgeTX/edgetx/releases
- **Discord:** https://discord.gg/wF9wUKnZ6H
- **Facebook:** https://www.facebook.com/groups/edgetx

## License

EdgeTX is licensed under the GNU General Public License v2.0. See [LICENSE](LICENSE) for details.

## Acknowledgements

EdgeTX is built on the shoulders of giants:
- **OpenTX** - The foundation upon which EdgeTX is built
- **FreeRTOS** - Real-time operating system
- **Lua** - Scripting language
- **Qt** - Companion application framework
- **STMicroelectronics** - HAL drivers
- **Community Contributors** - Hundreds of developers worldwide

## Support the Project

- **Contribute Code:** Submit pull requests
- **Report Issues:** Help improve quality
- **Write Documentation:** Share your knowledge
- **Help Users:** Answer questions on Discord
- **Donate:** https://opencollective.com/edgetx

---

**Welcome to EdgeTX! Together we're building the cutting edge of RC firmware.** 🚀
