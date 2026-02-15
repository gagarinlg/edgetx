# EdgeTX Developer Guide

Comprehensive guide for EdgeTX developers covering code structure, debugging, testing, and best practices.

## Table of Contents
- [Code Structure Overview](#code-structure-overview)
- [Development Environment Setup](#development-environment-setup)
- [Debugging Techniques](#debugging-techniques)
- [Testing Framework](#testing-framework)
- [Common Development Tasks](#common-development-tasks)
- [Performance Optimization](#performance-optimization)
- [Best Practices](#best-practices)

## Code Structure Overview

### Firmware Source Organization

```
radio/src/
├── main.cpp                 # Firmware entry point
├── mixer.cpp/h             # Mixer engine core
├── audio.cpp/h             # Audio system
├── telemetry/              # Telemetry protocols
├── lua/                    # Lua scripting engine
├── storage/                # Model/settings persistence
├── gui/                    # User interface layers
│   ├── 128x64/            # Monochrome display UI
│   ├── 212x64/            # Grayscale display UI
│   └── colorlcd/          # Color LCD UI
├── boards/                 # Board-specific HAL
│   ├── generic_stm32/     # Common STM32 code
│   ├── sky9x/             # Sky9x board
│   └── ...
├── targets/                # Radio target definitions
│   ├── horus/             # FrSky Horus family
│   ├── taranis/           # FrSky Taranis family
│   └── ...
├── thirdparty/            # External libraries
│   ├── CMSIS/             # ARM CMSIS
│   ├── STM32*_HAL_Driver/ # STM32 HAL
│   ├── Lua/               # Lua interpreter
│   └── lz4/               # LZ4 compression
├── pulses/                # RF protocol implementations
├── fonts/                 # Display fonts
└── bitmaps/               # UI graphics
```

### Companion Source Organization

```
companion/src/
├── main.cpp                      # Application entry
├── mainwindow.cpp/h             # Main window
├── appdata.cpp/h                # Data model layer
├── eeprominterface.cpp/h        # Firmware abstraction
├── firmwares/                   # Firmware definitions
│   └── edgetx/                  # EdgeTX specific
├── modeledit/                   # Model editor UI
├── generaledit/                 # Radio settings UI
├── simulation/                  # Simulator integration
├── storage/                     # File I/O
├── downloads/                   # Firmware downloads
├── flashfirmwaredialog.*        # Flashing interface
└── translations/                # Internationalization
```

## Development Environment Setup

### IDE Configuration

#### Visual Studio Code

**Extensions:**
- C/C++ (Microsoft)
- CMake Tools
- GitLens
- clang-format

**Settings** (`.vscode/settings.json`):
```json
{
  "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
  "cmake.configureOnOpen": true,
  "cmake.buildDirectory": "${workspaceFolder}/build-default",
  "editor.formatOnSave": true,
  "C_Cpp.clang_format_style": "file"
}
```

**Launch Configuration** (`.vscode/launch.json`):
```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug Simulator",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build-default/native/simulator",
      "args": [],
      "cwd": "${workspaceFolder}",
      "MIMode": "gdb"
    }
  ]
}
```

#### CLion

1. Open project directory
2. CLion auto-detects CMake
3. Select build configuration from dropdown
4. Set breakpoints and debug

#### Qt Creator

1. Open `CMakeLists.txt` as project
2. Configure kit (Qt + compiler)
3. Build target: `companion` or `simulator`
4. Run/Debug from IDE

### Command Line Tools

**Essential:**
```bash
# Code formatting
clang-format -i file.cpp

# Find TODO/FIXME comments
grep -rn "TODO\|FIXME" radio/src/

# Code statistics
cloc radio/src/

# Find large functions
# (install cflow: apt-get install cflow)
cflow -l radio/src/*.cpp | head -20
```

## Debugging Techniques

### Simulator Debugging

The simulator is the easiest way to debug firmware logic.

**Build debug simulator:**
```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Debug
cmake --build build-default --target simulator
```

**Run with debugger (GDB):**
```bash
gdb ./build-default/native/simulator
(gdb) break mixer.cpp:123
(gdb) run
(gdb) print variableName
(gdb) continue
```

**Run with debugger (LLDB on macOS):**
```bash
lldb ./build-default/native/simulator
(lldb) breakpoint set --file mixer.cpp --line 123
(lldb) run
(lldb) print variableName
(lldb) continue
```

**Debug output:**
```cpp
// Add to firmware code
#include "debug.h"

TRACE("Value: %d", myValue);
TRACE_WARNING("Unexpected state");
TRACE_ERROR("Failed: %s", errorMsg);
```

### Hardware Debugging

#### JTAG/SWD Debugging

**Hardware Required:**
- ST-Link V2/V3
- SWD connector (varies by radio)

**Connection:**
1. Connect ST-Link to radio's SWD pins (SWDIO, SWCLK, GND, 3.3V)
2. Connect ST-Link to PC via USB

**Using OpenOCD:**

Install:
```bash
sudo apt-get install openocd
```

Configure (`openocd.cfg`):
```
source [find interface/stlink.cfg]
source [find target/stm32f4x.cfg]

init
reset init
```

Run OpenOCD:
```bash
openocd -f openocd.cfg
```

In another terminal, connect GDB:
```bash
arm-none-eabi-gdb build-default/arm-none-eabi/firmware/firmware-X10.elf
(gdb) target remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) break main
(gdb) continue
```

#### Serial Debug Output

Some radios support serial output via USB or UART.

**Enable serial debug:**
```bash
cmake --preset default -DPCB=X10 -DDEBUG=YES -DTRACE_SD_CARD=YES
cmake --build build-default --target firmware-X10
```

**View output:**
```bash
# Linux
screen /dev/ttyUSB0 115200

# or
minicom -D /dev/ttyUSB0 -b 115200

# Windows: Use PuTTY or TeraTerm
```

#### LED Debugging

When debug output isn't available, use LED indicators:

```cpp
// In firmware code
#include "board.h"

// Blink LED
void debugBlink() {
  for (int i = 0; i < 3; i++) {
    LED_RED_ON();
    delay_ms(100);
    LED_RED_OFF();
    delay_ms(100);
  }
}

// Use in code
debugBlink(); // Visual confirmation
```

### Telemetry Debugging

Use telemetry to debug issues in the field:

```cpp
// Send debug value via telemetry
telemetryData.value1 = debugValue;
telemetryData.value2 = errorCode;
```

View on radio telemetry screen or in simulator.

## Testing Framework

### Unit Tests (GoogleTest)

EdgeTX uses GoogleTest for unit testing.

**Test Location:** `radio/src/tests/`

**Run tests:**
```bash
cmake --build build-default --target gtests-radio
./build-default/native/gtests-radio
```

**Run specific test:**
```bash
./build-default/native/gtests-radio --gtest_filter=MixerTest.BasicCurve
```

**Writing Tests:**

```cpp
// In radio/src/tests/mixer_test.cpp
#include "gtest/gtest.h"
#include "opentx.h"

TEST(MixerTest, LinearCurve) {
  // Setup
  g_model.mixData[0].weight = 100;
  g_model.mixData[0].srcRaw = MIXSRC_Rud;
  
  // Execute
  doMixerCalculations();
  
  // Verify
  EXPECT_EQ(channelOutputs[0], expectedValue);
}

TEST(MixerTest, ExponentialCurve) {
  // Test exponential curves
  g_model.expoData[0].expo = 50;
  
  int result = applyExpo(512, 50);
  
  EXPECT_GT(result, 512); // Should be higher than input
}
```

**Test Fixtures:**

```cpp
class MixerTestFixture : public ::testing::Test {
protected:
  void SetUp() override {
    // Initialize model data
    memset(&g_model, 0, sizeof(g_model));
  }
  
  void TearDown() override {
    // Cleanup
  }
};

TEST_F(MixerTestFixture, TestWithFixture) {
  // Test uses SetUp/TearDown
}
```

### Integration Testing

**Simulator-based Testing:**

1. Build simulator
2. Load test model
3. Exercise functionality
4. Verify outputs/behavior

**Manual Test Checklist:**
- [ ] Stick inputs read correctly
- [ ] Switches register state changes
- [ ] Mixer outputs expected values
- [ ] Audio plays correctly
- [ ] Telemetry displays properly
- [ ] SD card operations work
- [ ] Settings save/load correctly

### Companion Testing

**Run Companion tests:**
```bash
cmake --build build-default --target companion-tests
./build-default/native/companion-tests
```

**Manual Testing Areas:**
- Model import/export
- Firmware flashing
- Simulator integration
- Backup/restore
- Multiple radio types
- Settings migration

## Common Development Tasks

### Adding a New Mixer Feature

1. **Define data structure** (`radio/src/datastructs_private.h`):
```cpp
struct MixData {
  // ... existing fields
  int8_t myNewFeature; // Add new field
};
```

2. **Update storage** (`radio/src/storage/conversions/*.cpp`):
```cpp
// Add conversion for new field
if (version < NEW_VERSION) {
  mixData.myNewFeature = DEFAULT_VALUE;
}
```

3. **Implement logic** (`radio/src/mixer.cpp`):
```cpp
void evalMixes(uint8_t tick) {
  // ... existing code
  
  // Use new feature
  if (mix->myNewFeature) {
    value = applyMyNewFeature(value, mix->myNewFeature);
  }
}
```

4. **Add UI** (e.g., `radio/src/gui/colorlcd/model_mixes.cpp`):
```cpp
// Add edit control
new NumberEdit(&grid, {x, y}, -100, 100, GET_SET_DEFAULT(mix->myNewFeature));
```

5. **Test in simulator**
6. **Add unit tests**
7. **Update documentation**

### Adding a New Radio Target

See [HARDWARE_SUPPORT.md](HARDWARE_SUPPORT.md) for detailed instructions.

**Quick overview:**

1. Create target directory: `radio/targets/mynewradio/`
2. Copy reference target (similar radio)
3. Update board files: `radio/src/boards/myboard/`
4. Configure `CMakeLists.txt`
5. Add to `fw.json`
6. Test build

### Adding a GUI Screen

**For Color LCD:**

1. **Create header** (`radio/src/gui/colorlcd/my_screen.h`):
```cpp
#pragma once
#include "page.h"

class MyScreen : public Page {
public:
  MyScreen();
  
protected:
  void buildBody(FormWindow* window) override;
};
```

2. **Implement** (`radio/src/gui/colorlcd/my_screen.cpp`):
```cpp
#include "my_screen.h"
#include "widgets/text_edit.h"

MyScreen::MyScreen() : Page(ICON_MODEL) {
  header.setTitle("My Screen");
}

void MyScreen::buildBody(FormWindow* window) {
  FormGridLayout grid;
  grid.setLabelWidth(100);
  
  // Add widgets
  new StaticText(window, grid.getLabelSlot(), "Label:");
  new TextEdit(window, grid.getFieldSlot(), myData.text, 10);
  
  grid.nextLine();
}
```

3. **Register screen** (in menu or navigation code):
```cpp
new TextButton(&grid, {x, y}, "My Screen", [=]() {
  new MyScreen();
  return 0;
});
```

### Adding a Telemetry Protocol

1. **Create protocol handler** (`radio/src/telemetry/myprotocol.cpp`):
```cpp
void processMyProtocolPacket(uint8_t* data, uint8_t len) {
  // Parse packet
  uint16_t voltage = (data[0] << 8) | data[1];
  
  // Store in telemetry
  setTelemetryValue(PROTOCOL_TELEMETRY_VBAT, 0, 0, voltage, UNIT_VOLTS, 2);
}
```

2. **Register protocol** (`radio/src/telemetry/telemetry.cpp`):
```cpp
case PROTOCOL_MY_PROTOCOL:
  processMyProtocolPacket(packet, len);
  break;
```

3. **Add protocol definition** (`radio/src/dataconstants.h`):
```cpp
enum TelemetryProtocol {
  // ... existing
  PROTOCOL_MY_PROTOCOL,
};
```

## Performance Optimization

### Profiling

**Simulator profiling:**
```bash
# Build with profiling
cmake --preset default -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-default --target simulator

# Run with profiler
valgrind --tool=callgrind ./build-default/native/simulator
kcachegrind callgrind.out.*
```

**Hardware profiling:**
- Use DWT cycle counter (Cortex-M)
- Measure execution time of critical functions
- Use GPIO toggling + logic analyzer

**Example timing code:**
```cpp
// In critical function
uint32_t start = DWT->CYCCNT;

// ... code to measure ...

uint32_t cycles = DWT->CYCCNT - start;
TRACE("Function took %d cycles", cycles);
```

### Optimization Guidelines

**DO:**
- Profile before optimizing
- Optimize critical paths (mixer, audio ISR)
- Use `-O2` or `-O3` for release builds
- Enable LTO (Link Time Optimization)
- Use inline for small, frequently-called functions
- Minimize dynamic allocation
- Use const and constexpr

**DON'T:**
- Optimize prematurely
- Sacrifice readability for minor gains
- Ignore compiler warnings
- Use undefined behavior for speed

**Memory Optimization:**
```cpp
// Bad: Dynamic allocation in mixer
char* buffer = malloc(256);

// Good: Static or stack allocation
static char buffer[256];
// or
char buffer[256];
```

**Loop Optimization:**
```cpp
// Bad: Calculation in loop condition
for (int i = 0; i < strlen(str); i++)

// Good: Cache result
int len = strlen(str);
for (int i = 0; i < len; i++)
```

## Best Practices

### Code Quality

**1. Keep Functions Small:**
```cpp
// Bad: 200-line function doing everything

// Good: Split into logical units
void processInput() {
  readSticks();
  readSwitches();
  readPots();
}
```

**2. Use Meaningful Names:**
```cpp
// Bad
int x, y, z;
void f();

// Good
int channelValue, mixerResult;
void calculateMixerOutput();
```

**3. Comment Non-Obvious Code:**
```cpp
// Good: Explains WHY, not WHAT
// Apply exponential curve with midpoint adjustment
// to provide more control around center stick
result = applyExpo(input, expo, midpoint);
```

**4. Handle Errors:**
```cpp
// Bad
file = openFile(path);
file.write(data);

// Good
file = openFile(path);
if (file.isOpen()) {
  if (!file.write(data)) {
    TRACE_ERROR("Write failed");
  }
  file.close();
}
```

### Memory Management

**Stack vs Heap:**
```cpp
// Firmware: Prefer stack (heap is limited)
void myFunction() {
  char buffer[128]; // Stack - OK
  // vs
  char* buffer = malloc(128); // Heap - Avoid
}
```

**Static Allocation:**
```cpp
// For persistent data
static uint8_t audioBuffer[AUDIO_BUFFER_SIZE];
```

**RAII Pattern:**
```cpp
// Use RAII for resource management
class FileHandle {
  FILE* f;
public:
  FileHandle(const char* path) : f(fopen(path, "r")) {}
  ~FileHandle() { if (f) fclose(f); }
  FILE* get() { return f; }
};

// Auto-closes on scope exit
{
  FileHandle file("model.bin");
  if (file.get()) {
    // Use file
  }
} // Automatically closed
```

### Threading Considerations

**FreeRTOS Tasks:**
```cpp
// High-priority mixer task
void mixerTask(void* param) {
  while (1) {
    doMixerCalculations();
    vTaskDelay(MIXER_PERIOD_MS);
  }
}

// Create task
xTaskCreate(mixerTask, "mixer", STACK_SIZE, NULL, PRIORITY, &handle);
```

**Shared Data Protection:**
```cpp
// Use mutex for shared data
static SemaphoreHandle_t dataMutex;

void writeSharedData(int value) {
  xSemaphoreTake(dataMutex, portMAX_DELAY);
  sharedData = value;
  xSemaphoreGive(dataMutex);
}
```

### Debugging Best Practices

1. **Reproduce in Simulator First** (if possible)
2. **Use Version Control** - commit working states
3. **Add Assertions** for invariants
4. **Log State** at critical points
5. **Test Edge Cases** (min/max values, errors)
6. **Rubber Duck Debugging** - explain to someone/something

## Development Tips

### Quick Iteration

**Fast rebuild for single file change:**
```bash
# Only rebuilds changed files
cmake --build build-default --target firmware-X10
```

**Incremental simulator testing:**
1. Make code change
2. Rebuild simulator (fast)
3. Test in simulator
4. Iterate

### Code Navigation

**Find definition:**
```bash
grep -rn "void myFunction" radio/src/
```

**Find usage:**
```bash
grep -rn "myFunction(" radio/src/
```

**Find struct definition:**
```bash
grep -rn "struct ModelData" radio/src/
```

### Git Workflow

```bash
# Create feature branch
git checkout -b feature/my-feature

# Make changes
# ... edit files ...

# Check what changed
git diff

# Commit
git add file1.cpp file2.h
git commit -m "feat(mixer): add new feature"

# Keep updated
git fetch upstream
git rebase upstream/main

# Push
git push origin feature/my-feature
```

## Additional Resources

- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture
- [BUILD.md](BUILD.md) - Build instructions
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [API_REFERENCE.md](API_REFERENCE.md) - API documentation
- [Development Wiki](https://github.com/EdgeTX/edgetx/wiki)
- [Discord #development](https://discord.gg/wF9wUKnZ6H)

Happy coding! 🚀
