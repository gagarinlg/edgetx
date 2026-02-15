# EdgeTX Hardware Support Guide

Guide for adding support for new radio hardware to EdgeTX.

## Table of Contents
- [Overview](#overview)
- [Prerequisites](#prerequisites)
- [Understanding Target Structure](#understanding-target-structure)
- [Step-by-Step Guide](#step-by-step-guide)
- [Board Definition](#board-definition)
- [Target Configuration](#target-configuration)
- [Testing New Hardware](#testing-new-hardware)
- [Troubleshooting](#troubleshooting)

## Overview

Adding a new radio to EdgeTX involves:
1. Creating a **board definition** (hardware abstraction)
2. Creating a **target definition** (radio-specific config)
3. Implementing **drivers** for peripherals
4. Adding **build configuration**
5. Testing and validation

### Supported Hardware Platforms

EdgeTX currently supports:
- **STM32F2** series (Taranis X9D, X9D+)
- **STM32F4** series (Taranis X9E, X-Lite, Jumper T12/T16)
- **STM32F7** series (TBS Tango 2)
- **STM32H7** series (RadioMaster TX16S, Jumper T18/T20)
- **STM32H7RS** series (Next-gen radios)

## Prerequisites

### Knowledge Required
- C/C++ programming
- Embedded systems basics
- STM32 microcontroller architecture
- CMake build system
- Electronics/hardware interfacing

### Hardware Required
- Target radio or development board
- JTAG/SWD debugger (ST-Link V2/V3)
- Oscilloscope/logic analyzer (helpful)
- USB cable for programming

### Documentation Needed
- Radio schematic (if available)
- MCU datasheet
- Peripheral datasheets (LCD, audio codec, RF module, etc.)

## Understanding Target Structure

### File Organization

```
edgetx/
├── radio/
│   ├── src/
│   │   ├── boards/              # Board definitions (HAL)
│   │   │   ├── generic_stm32/  # Common STM32 code
│   │   │   ├── sky9x/          # Board: Sky9x
│   │   │   ├── taranis/        # Board: Taranis family
│   │   │   ├── horus/          # Board: Horus family
│   │   │   └── mynewboard/     # NEW: Your board
│   │   │
│   │   └── targets/            # Target definitions
│   │       ├── taranis/        # Target: Taranis radios
│   │       ├── horus/          # Target: Horus radios
│   │       └── mynewradio/     # NEW: Your target
│   │
│   └── util/
│       └── hw_defs/            # Hardware definition files
│
└── fw.json                     # Build configuration
```

### Board vs Target

**Board** (in `boards/`):
- Hardware abstraction layer (HAL)
- Peripheral drivers (LCD, keys, audio, ADC, etc.)
- Low-level hardware initialization
- Can support multiple radio targets

**Target** (in `targets/`):
- Radio-specific configuration
- Button/switch layout
- Screen type and resolution
- RF modules and protocols
- References a specific board

**Example:**
- **Board**: `horus` (common HAL for Horus hardware)
- **Targets**: `x10`, `x10express`, `x12s` (all use `horus` board)

## Step-by-Step Guide

### Step 1: Choose Reference Hardware

Find an existing radio similar to your new hardware:

| Your Hardware | Similar Existing | Board Base | Screen Type |
|---------------|------------------|------------|-------------|
| STM32F4 + mono LCD | Taranis X9D+ | taranis | 128x64 mono |
| STM32F4 + color LCD | X-Lite | taranis | 320x480 color |
| STM32H7 + large color | TX16S | horus | 480x272 color |

### Step 2: Create Board Definition

Create directory: `radio/src/boards/mynewboard/`

**Required files:**

1. **CMakeLists.txt** - Build configuration
2. **board.h** - Board interface definitions
3. **board.cpp** - Board initialization
4. **lcd_driver.cpp** - LCD driver
5. **keys_driver.cpp** - Button/switch driver
6. **audio_driver.cpp** - Audio driver
7. **adc_driver.cpp** - Analog input driver

**Example: `boards/mynewboard/CMakeLists.txt`**

```cmake
set(ARCH ARM)
set(MCU cortex-m7)  # or cortex-m4, cortex-m3
set(MCU_FLAGS "-mcpu=${MCU} -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")

set(STM32LIB_SRC_DIR ${RADIO_SRC_DIR}/thirdparty)
set(STM32_HAL_DRIVER_DIR ${STM32LIB_SRC_DIR}/STM32H7xx_HAL_Driver)

add_definitions(
  -DSTM32H743xx   # MCU definition
  -DUSE_HAL_DRIVER
  -DHSE_VALUE=12000000  # External crystal frequency
)

set(BOARD_INCLUDES
  ${RADIO_SRC_DIR}/boards/generic_stm32
  ${RADIO_SRC_DIR}/boards/${BOARD_NAME}
  ${STM32_HAL_DRIVER_DIR}/Inc
  # ... other includes
)

set(BOARD_SRC
  ${BOARD_SRC}
  board.cpp
  lcd_driver.cpp
  keys_driver.cpp
  audio_driver.cpp
  adc_driver.cpp
  # ... other sources
)

# Include HAL files
include(${RADIO_SRC_DIR}/boards/generic_stm32/stm32h7_hal.cmake)
```

**Example: `boards/mynewboard/board.h`**

```cpp
#ifndef BOARD_H
#define BOARD_H

#include "stm32h7xx.h"

// Board identification
#define BOARD_NAME "mynewboard"

// LCD Configuration
#define LCD_W 480
#define LCD_H 272
#define LCD_DEPTH 16  // bits per pixel

// LED Configuration
#define LED_RED_GPIO GPIOA
#define LED_RED_PIN GPIO_PIN_0
#define LED_GREEN_GPIO GPIOA
#define LED_GREEN_PIN GPIO_PIN_1

// LED Macros
#define LED_RED_ON() HAL_GPIO_WritePin(LED_RED_GPIO, LED_RED_PIN, GPIO_PIN_SET)
#define LED_RED_OFF() HAL_GPIO_WritePin(LED_RED_GPIO, LED_RED_PIN, GPIO_PIN_RESET)

// Keys/Switches
enum EnumKeys {
  KEY_MENU,
  KEY_EXIT,
  KEY_ENTER,
  KEY_UP,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT,
  TRM_BASE,
  // ... define all keys
  NUM_KEYS
};

// ADC Configuration
#define NUM_STICKS 4
#define NUM_POTS 2
#define NUM_SLIDERS 2
#define NUM_ANALOG_INPUTS (NUM_STICKS + NUM_POTS + NUM_SLIDERS)

// Audio Configuration
#define AUDIO_SAMPLE_RATE 32000

// SD Card
#define SD_PRESENT_GPIO GPIOC
#define SD_PRESENT_PIN GPIO_PIN_13

// USB Configuration
#define USB_DP_GPIO GPIOA
#define USB_DP_PIN GPIO_PIN_12
#define USB_DM_GPIO GPIOA
#define USB_DM_PIN GPIO_PIN_11

// Function declarations
void boardInit();
void boardOff();

#endif // BOARD_H
```

**Example: `boards/mynewboard/board.cpp`**

```cpp
#include "board.h"
#include "hal.h"

void boardInit()
{
  // Configure system clock
  SystemClock_Config();
  
  // Initialize HAL
  HAL_Init();
  
  // GPIO Clocks
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  // Configure LEDs
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = LED_RED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_RED_GPIO, &GPIO_InitStruct);
  
  // Initialize peripherals
  lcdInit();
  keysInit();
  audioInit();
  adcInit();
  
  // Initialize SD card
  sdInit();
  
  // Initialize USB
  usbInit();
}

void boardOff()
{
  // Power down peripherals
  lcdOff();
  audioOff();
  
  // Enter low power mode
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}
```

### Step 3: Implement Peripheral Drivers

#### LCD Driver (`lcd_driver.cpp`)

```cpp
#include "board.h"
#include "lcd.h"

static uint16_t* lcdBuffer = nullptr;

void lcdInit()
{
  // Allocate framebuffer
  lcdBuffer = (uint16_t*)malloc(LCD_W * LCD_H * sizeof(uint16_t));
  
  // Configure LTDC (LCD controller)
  // ... STM32-specific LTDC setup
  
  // Configure DMA2D (for acceleration)
  // ... STM32-specific DMA2D setup
}

void lcdRefresh()
{
  // Transfer framebuffer to LCD
  // Usually handled by LTDC in background
}

void lcdDrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  if (x < LCD_W && y < LCD_H) {
    lcdBuffer[y * LCD_W + x] = color;
  }
}

void lcdClear(uint16_t color)
{
  for (int i = 0; i < LCD_W * LCD_H; i++) {
    lcdBuffer[i] = color;
  }
}
```

#### Keys Driver (`keys_driver.cpp`)

```cpp
#include "board.h"
#include "keys.h"

uint32_t readKeys()
{
  uint32_t result = 0;
  
  // Read GPIO pins for each key
  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) {
    result |= (1 << KEY_MENU);
  }
  
  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET) {
    result |= (1 << KEY_EXIT);
  }
  
  // ... read all keys
  
  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;
  
  // Read trim switches
  // ... similar to keys
  
  return result;
}
```

#### ADC Driver (`adc_driver.cpp`)

```cpp
#include "board.h"
#include "adc.h"

static uint16_t adcValues[NUM_ANALOG_INPUTS];

void adcInit()
{
  // Configure ADC
  ADC_HandleTypeDef hadc;
  hadc.Instance = ADC1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  // ... configure ADC
  
  HAL_ADC_Init(&hadc);
  
  // Start DMA-based continuous conversion
  HAL_ADC_Start_DMA(&hadc, (uint32_t*)adcValues, NUM_ANALOG_INPUTS);
}

uint16_t getAnalogValue(uint8_t index)
{
  if (index < NUM_ANALOG_INPUTS) {
    return adcValues[index];
  }
  return 0;
}
```

### Step 4: Create Target Definition

Create directory: `radio/src/targets/mynewradio/`

**Required files:**

1. **CMakeLists.txt** - Target build config
2. **hal.h** - Hardware configuration
3. **board_defs.json** - Board definition (optional)

**Example: `targets/mynewradio/CMakeLists.txt`**

```cmake
set(TARGET_DIR mynewradio)

option(DISK_CACHE "Enable SD card disk cache" YES)
option(UNEXPECTED_SHUTDOWN "Enable unexpected shutdown detection" YES)

set(PWR_BUTTON "PRESS" CACHE STRING "Power button type")
set(CPU_TYPE STM32H7)
set(BOARD_NAME mynewboard)
set(LINKER_SCRIPT targets/${TARGET_DIR}/linker_script.ld)

add_definitions(
  -DPCBMYNEWRADIO
  -DRADIO_FAMILY_MYNEWRADIO
)

# Screen configuration
if(LCD_COLORLCD)
  set(GUI_DIR colorlcd)
  add_definitions(-DCOLORLCD)
endif()

# Hardware options
set(STORAGE_MODELSLIST YES)
set(HARDWARE_INTERNAL_MODULE YES)
set(HARDWARE_EXTERNAL_MODULE YES)

# Include target sources
set(TARGET_SRC
  ${TARGET_SRC}
  board.cpp
  # ... target-specific sources
)

# Include board definition
include(${RADIO_SRC_DIR}/boards/${BOARD_NAME}/board.cmake)
```

**Example: `targets/mynewradio/hal.h`**

```cpp
#ifndef HAL_H
#define HAL_H

// Radio information
#define PCBMYNEWRADIO
#define FLAVOUR "mynewradio"

// Display
#define LCD_W 480
#define LCD_H 272
#define LCD_DEPTH 16

// Storage
#define EEPROM_SIZE 4096
#define MODEL_DATA_SIZE 16384

// RF Modules
#define NUM_MODULES 2
#define INTERNAL_MODULE_MULTI
#define EXTERNAL_MODULE_ENABLED

// Hardware features
#define HAPTIC
#define BLUETOOTH
#define USB_CHARGER

// Audio
#define AUDIO_SPEAKER_ENABLE_GPIO GPIOD, GPIO_PIN_13

// Timers
#define MIXER_SCHEDULER_TIMER TIM1
#define TIMER_10MS_IRQHandler TIM1_CC_IRQHandler

#endif // HAL_H
```

### Step 5: Add to Build System

Edit `fw.json` to add your target:

```json
{
  "targets": [
    {
      "name": "mynewradio",
      "pcb": "MYNEWRADIO",
      "description": "My New Radio",
      "manufacturer": "MyManufacturer",
      "board": "mynewboard",
      "cpu": "STM32H743",
      "features": [
        "colorlcd",
        "bluetooth",
        "internal_multi",
        "lua"
      ],
      "options": {
        "PCB": "MYNEWRADIO",
        "DEFAULT_MODE": "2",
        "LUA": "YES",
        "MULTIMODULE": "YES"
      }
    }
  ]
}
```

### Step 6: Create Linker Script

Create `targets/mynewradio/linker_script.ld`:

```ld
/* STM32H743 Memory Layout */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 2048K
  RAM (rwx)       : ORIGIN = 0x20000000, LENGTH = 512K
  SDRAM (rwx)     : ORIGIN = 0xD0000000, LENGTH = 8M  /* External RAM if available */
}

/* Entry point */
ENTRY(Reset_Handler)

SECTIONS
{
  .text :
  {
    . = ALIGN(4);
    *(.isr_vector)
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
    _etext = .;
  } > FLASH

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > RAM AT> FLASH

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > RAM

  /* Heap and stack */
  .heap :
  {
    . = ALIGN(4);
    _heap_start = .;
    . = . + 32K;  /* Heap size */
    _heap_end = .;
  } > RAM

  .stack :
  {
    . = ALIGN(8);
    . = . + 4K;  /* Stack size */
    _estack = .;
  } > RAM
}
```

## Board Definition

### Hardware Definition File

Create `radio/util/hw_defs/mynewradio.json`:

```json
{
  "name": "mynewradio",
  "board": "mynewboard",
  "mcu": "STM32H743",
  
  "pins": {
    "LCD_RST": "PD3",
    "LCD_BL": "PD12",
    "SD_CS": "PC11",
    "AUDIO_AMP_EN": "PD13"
  },
  
  "adc_channels": {
    "STICK_LH": "ADC1_CH0",
    "STICK_LV": "ADC1_CH1",
    "STICK_RH": "ADC1_CH2",
    "STICK_RV": "ADC1_CH3",
    "POT1": "ADC1_CH4",
    "POT2": "ADC1_CH5"
  },
  
  "keys": [
    {"name": "KEY_MENU", "pin": "PB0"},
    {"name": "KEY_EXIT", "pin": "PB1"},
    {"name": "KEY_ENTER", "pin": "PB2"}
  ],
  
  "switches": [
    {"name": "SW_SA", "pins": ["PA0", "PA1", "PA2"]},
    {"name": "SW_SB", "pins": ["PA3", "PA4"]}
  ]
}
```

## Testing New Hardware

### Phase 1: Bootloader Testing

1. **Build bootloader:**
   ```bash
   cmake --build build-default --target bootloader-mynewradio
   ```

2. **Flash via JTAG/SWD**

3. **Test:**
   - Power on (LED should indicate boot)
   - USB connection (should enumerate)
   - DFU mode entry

### Phase 2: Basic Hardware Testing

1. **Build minimal firmware:**
   ```bash
   cmake --preset default -DPCB=MYNEWRADIO -DLUA=NO
   cmake --build build-default --target firmware-mynewradio
   ```

2. **Test sequence:**
   - [ ] Power on/off
   - [ ] LED indicators
   - [ ] LCD display (show test pattern)
   - [ ] Buttons/switches (log presses)
   - [ ] Analog inputs (display raw values)
   - [ ] Audio (play test tone)
   - [ ] SD card (mount/read/write)
   - [ ] USB (enumerate, file transfer)

### Phase 3: Firmware Integration

1. **Build full firmware**

2. **Test features:**
   - [ ] Model creation
   - [ ] Mixer functionality
   - [ ] Telemetry display
   - [ ] Lua scripts
   - [ ] RF module communication
   - [ ] Trainer port
   - [ ] Bluetooth (if supported)

### Phase 4: Stress Testing

- Long-term operation
- Rapid input changes
- Thermal testing
- Power cycling
- Low battery conditions

## Troubleshooting

### Common Issues

**LCD not working:**
- Check pin configuration
- Verify LTDC/SPI setup
- Test with simple pattern (all white, all black)
- Use logic analyzer to verify signals

**Keys not responding:**
- Check pull-up/pull-down configuration
- Verify GPIO clock enabled
- Test with multimeter (voltage levels)

**ADC values incorrect:**
- Check reference voltage
- Verify channel mapping
- Calibrate ADC
- Check for noise (use scope)

**Audio not playing:**
- Check amplifier enable pin
- Verify DMA configuration
- Test DAC output with scope
- Check sample rate

**SD card not detected:**
- Verify SPI/SDIO configuration
- Check card detect pin
- Test with different card brands
- Verify voltage levels (3.3V)

**USB not enumerating:**
- Check D+/D- connections
- Verify clock configuration (48MHz)
- Test with different cables
- Check USB descriptor

### Debug Tools

**Hardware:**
```cpp
// Blink LED to verify boot
void debugBlink(int count) {
  for (int i = 0; i < count; i++) {
    LED_RED_ON();
    HAL_Delay(100);
    LED_RED_OFF();
    HAL_Delay(100);
  }
}

// Call in various places to trace execution
debugBlink(1); // After clock init
debugBlink(2); // After GPIO init
debugBlink(3); // After LCD init
```

**Serial Output:**
```cpp
// Send debug via UART
void debugPrint(const char* msg) {
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
}
```

## Submitting New Hardware Support

### Before Submitting

1. **Test thoroughly** on actual hardware
2. **Document** hardware specifics
3. **Clean code** (follow coding standards)
4. **Add to build system** (fw.json)
5. **Create PR** with description

### PR Checklist

- [ ] Code compiles without warnings
- [ ] Hardware fully functional
- [ ] Documentation updated
- [ ] Build system integration complete
- [ ] Photos/screenshots included
- [ ] Tested by multiple people (if possible)

## Additional Resources

- [STM32 Reference Manuals](https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html)
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture
- [BUILD.md](BUILD.md) - Build instructions
- [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) - Development guide
- [EdgeTX Discord](https://discord.gg/wF9wUKnZ6H) - #hardware channel

Good luck with your new hardware port! 🎉
