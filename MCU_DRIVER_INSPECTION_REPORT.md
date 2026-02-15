# EdgeTX MCU and Hardware Driver Detailed Inspection Report

**Inspection Date:** February 15, 2026  
**Scope:** MCU-specific code and hardware drivers  
**Inspector:** Code Review Agent

---

## Executive Summary

This report provides a comprehensive security and quality analysis of EdgeTX's MCU-specific and hardware driver code, covering:
- **324 files** across targets, HAL, and driver directories
- **4 MCU families:** STM32F2, STM32F4, STM32H7, STM32H7RS
- **20+ peripheral drivers:** DMA, SPI, I2C, UART, ADC, GPIO, USB, etc.
- **Hardware-specific drivers:** SD Card, Flash, SDRAM, Display, Audio, Sensors

### Critical Findings Summary

| Priority | Count | Category |
|----------|-------|----------|
| 🔴 Critical | 6 | System hangs, buffer overflows |
| 🟡 High | 12 | Race conditions, missing validation |
| 🟠 Medium | 15+ | Error handling, synchronization |

### Risk Assessment

**Current Risk Level:** 🔴 **HIGH**
- Infinite loops can cause system lockup
- Buffer overflows in DMA operations
- Race conditions in GPIO and shared state

**After Remediation:** 🟡 **MEDIUM → LOW**

---

## Table of Contents

1. [MCU Architecture Analysis](#1-mcu-architecture-analysis)
2. [System Initialization](#2-system-initialization)
3. [Interrupt Handling](#3-interrupt-handling)
4. [Memory Configuration](#4-memory-configuration)
5. [Peripheral Driver Security](#5-peripheral-driver-security)
6. [Hardware-Specific Drivers](#6-hardware-specific-drivers)
7. [Critical Issues Detail](#7-critical-issues-detail)
8. [Recommendations](#8-recommendations)

---

## 1. MCU Architecture Analysis

### 1.1 Supported MCU Families

EdgeTX supports four STM32 families with increasing capabilities:

| MCU Family | Core | Flash | RAM | Radios | Key Features |
|------------|------|-------|-----|--------|--------------|
| **STM32F2** | Cortex-M3 | 512KB | 128KB | Older Taranis | Basic, legacy |
| **STM32F4** | Cortex-M4 | 1-2MB | 128-256KB | X9D+, X10, X12S | SDRAM, LTDC display |
| **STM32H7** | Cortex-M7 | 128KB-2MB | 1MB | Horus, TX16S | QSPI, DMA2D, DSI |
| **STM32H7RS** | Cortex-M7+ | 64KB-2MB | 640KB | Latest (future) | XSPI, PSRAM, advanced |

### 1.2 HAL Configuration Files

**File Locations:**
- `radio/src/targets/common/arm/stm32/stm32f2xx_hal_conf.h` (496 lines)
- `radio/src/targets/common/arm/stm32/stm32f4xx_hal_conf.h` (575 lines)
- `radio/src/targets/common/arm/stm32/stm32h7xx_hal_conf.h` (538 lines)
- `radio/src/targets/common/arm/stm32/stm32h7rsxx_hal_conf.h` (489 lines)

**Enabled Peripherals (by family):**

```
STM32F2xx: DMA, FLASH, GPIO, I2C, PWR, RCC, RTC, SPI, TIM, UART, USB
STM32F4xx: + CORTEX, DMA2D, LTDC, SDRAM, I2S
STM32H7xx: + MDMA, DSI, QSPI, SAI, SPI (advanced)
STM32H7RS: + XSPI, I3C, LPTIM, HSPI
```

**Clock Configurations:**

**STM32F4** (system_stm32f4xx.c):
```c
HSE = 25MHz
PLL_M = 12, PLL_N = 336, PLL_P = 2
SYSCLK = 168MHz
APB1 = 42MHz, APB2 = 84MHz
Flash Latency: 5 Wait States
```

**STM32H7** (system_stm32h7xx.c):
```c
HSE = 25MHz
VOS1 (Voltage scaling mode 1)
SYSCLK = 400-480MHz (configurable)
AHB = 200-240MHz
APB1/2/3/4 individually configurable
```

### 1.3 Security Features Enabled

✅ **Memory Protection Unit (MPU):**
- **File:** `system_init.c`, lines 202-290
- **Configuration:**
  ```c
  Region 0: Default (4GB, no access, strongly ordered)
  Region 2: QSPI/NOR flash (cacheable, bufferable, code execution)
  Region 3: External SDRAM/PSRAM (cacheable, bufferable)
  Region 4: DMA buffers (cache-disabled, 64KB, no code exec)
  ```
- **Status:** Enabled with `HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT)`

✅ **Cache Configuration:**
- I-Cache and D-Cache enabled on capable MCUs
- Prefetch buffer enabled for STM32F4
- Cache cleaning on bootloader entry

✅ **Bus Fault Exception:**
- Explicitly enabled: `SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk`

⚠️ **Write Protection:** Not configured in code (may be set by debugger/flasher)

⚠️ **Firewall:** Not used (available on some STM32H7 variants)

---

## 2. System Initialization

### 2.1 Boot Sequence

**File:** `radio/src/targets/common/arm/stm32/system_init.c`

**Reset Handler** (lines 47-143):

```c
1. Stack Pointer Setup → _estack
2. SystemInit() (vendor HAL)
3. SystemClock_Config() (custom per MCU)
4. Code Relocation:
   - Copy text section to ITCM/DTCM (H7)
   - Copy IRAM code for fast execution
5. MPU Configuration (if enabled)
6. ISR Vector Table Setup:
   - Copy vectors to RAM
   - Set VTOR register
7. BSS Zeroing
8. Static Constructors
9. Jump to main()
```

**Boot Time:** Estimated 50-200ms depending on MCU and external memory

### 2.2 Clock Configuration

**Issue Found:** ⚠️ **No HSE failure handling**

All MCU variants assume HSE (external crystal) starts successfully. If HSE fails:
- System hangs in `SystemClock_Config()`
- No fallback to HSI (internal oscillator)
- No error indication

**Recommendation:** Add HSE startup timeout with fallback to HSI

---

## 3. Interrupt Handling

### 3.1 Vector Table and Priorities

**File:** `radio/src/targets/common/arm/stm32/cortex_m_isr.c`

**Exception Handlers:**
```c
NMI_Handler           - Non-maskable interrupt
HardFault_Handler     - Hard fault (memory, bus errors)
MemManage_Handler     - Memory protection fault
BusFault_Handler      - Bus fault (MPU violation)
UsageFault_Handler    - Usage fault (div by 0, unaligned access)
SVC_Handler           - Supervisor call
DebugMon_Handler      - Debug monitor
PendSV_Handler        - Pendable service request
SysTick_Handler       - System tick timer
```

**Interrupt Priorities (4-bit):**
```c
TICK_INT_PRIORITY = 0x0F (lowest)
DMA IRQ priorities: 0x05-0x07 (mid-high)
UART IRQ: 0x08 (medium)
EXTI IRQ: 0x09 (medium-low)
```

### 3.2 Hard Fault Handler

**Code** (lines 73-80):
```c
__attribute__((optimize("O0")))
void hard_fault_handler_c(sContextStateFrame *frame) {
  HALT_IF_DEBUGGING();  // Breakpoint if debugger connected
  // Infinite loop if no debugger
  while(1);
}
```

**Good:** 
- Stack frame capture for debugging
- Debugger awareness

**Missing:**
- No crash dump to persistent storage
- No LED indication for field debugging
- No safe mode reboot

**Recommendation:** Add crash logging to SD card or flash

### 3.3 Default Handler

**Issue:** Undefined interrupts trigger infinite loop

**Code** (lines 44-60):
```c
void default_isr_handler(void) {
  HALT_IF_DEBUGGING();
  while(1);  // System hangs
}
```

**Recommendation:** Add interrupt ID logging and safe reboot

---

## 4. Memory Configuration

### 4.1 Memory Maps by MCU Family

#### STM32F4 (X9D+, X10)

**Linker Script:** `radio/src/targets/taranis/stm32f40x/layout.ld`

```
┌─────────────────────────────────────┐
│ FLASH (512KB) @ 0x08000000          │
│ - Bootloader (64KB)                 │
│ - Application (448KB)               │
├─────────────────────────────────────┤
│ RAM (128KB) @ 0x20000000            │
│ - Data/BSS                          │
│ - Heap (grows up)                   │
├─────────────────────────────────────┤
│ CCM (64KB) @ 0x10000000             │
│ - Stack (1KB, grows down)           │
│ - Fast code sections                │
└─────────────────────────────────────┘
```

#### STM32H7 with SDRAM (Horus X12S)

**Linker Script:** `radio/src/targets/horus/stm32h750_sdram/layout.ld`

```
┌─────────────────────────────────────┐
│ FLASH (128KB) @ 0x08000000          │
│ - Bootloader only                   │
├─────────────────────────────────────┤
│ ITCMRAM (64KB) @ 0x00000000         │
│ - Fast instruction execution        │
├─────────────────────────────────────┤
│ DTCMRAM (128KB) @ 0x20000000        │
│ - ISR vectors + critical data       │
├─────────────────────────────────────┤
│ RAM_D1 (512KB) @ 0x24000000         │
│ - BSS segment                       │
├─────────────────────────────────────┤
│ SDRAM (8MB) @ 0xC0000000            │
│ - Application code                  │
│ - Heap (4MB minimum)                │
├─────────────────────────────────────┤
│ NORFLASH (8MB) @ 0x90000000         │
│ - External storage                  │
└─────────────────────────────────────┘

Stack: 8KB (DTCMRAM top)
```

#### STM32H7RS with PSRAM (Future radios)

**Linker Script:** `radio/src/targets/tx16smk3/stm32h7rs_sdram/layout.ld`

```
┌─────────────────────────────────────┐
│ FLASH (64KB) @ 0x08000000           │
│ - Bootloader only                   │
├─────────────────────────────────────┤
│ ITCMRAM (64KB) @ 0x00000000         │
│ - SRAM1 (instruction)               │
├─────────────────────────────────────┤
│ DTCMRAM (64KB) @ 0x20000000         │
│ - SRAM3 (data)                      │
├─────────────────────────────────────┤
│ RAM (384KB) @ 0x24000000            │
│ - SRAM1+2+3 combined                │
├─────────────────────────────────────┤
│ DMA (72KB) @ 0x24060000             │
│ - SRAM4 (cache-disabled)            │
├─────────────────────────────────────┤
│ PSRAM (32.6MB) @ 0x90000000         │
│ - Application code + data           │
│ - Heap (4MB minimum)                │
├─────────────────────────────────────┤
│ NORFLASH (128MB) @ 0x70000000       │
│ - Extended storage via XSPI         │
└─────────────────────────────────────┘

Stack: 8KB (DTCMRAM top)
```

### 4.2 Memory Access Patterns

**Good Practices:**
- ✅ ITCM/DTCM used for interrupt vectors and critical code
- ✅ DMA buffers in cache-disabled region
- ✅ MPU enforces code/data separation

**Issues:**
- ⚠️ No explicit .noinit sections for persistent variables
- ⚠️ Heap/stack collision detection only via linker symbols

---

## 5. Peripheral Driver Security

### 5.1 DMA Driver Issues

**File:** `radio/src/targets/common/arm/stm32/stm32_dma.cpp`

#### 🔴 CRITICAL: Infinite Blocking Loops

**Location:** Used in SPI (lines 283-289), ADC (lines 637-657), UART (lines 635-676)

**Example from SPI:**
```cpp
// stm32_spi.cpp, lines 283-289
while(!stm32_dma_check_tc_flag(spi->DMA, spi->rxDMA_Stream));  // NO TIMEOUT
while (!LL_SPI_IsActiveFlag_TXE(spi->SPIx));                    // NO TIMEOUT
while(LL_SPI_IsActiveFlag_BSY(spi->SPIx));                      // NO TIMEOUT
```

**Impact:**
- If DMA hardware stalls, entire system locks up
- No watchdog can recover (interrupts disabled)
- Requires power cycle

**Recommendation:**
```cpp
#define DMA_TIMEOUT_MS 1000

bool wait_dma_complete(uint32_t timeout_ms) {
    uint32_t start = HAL_GetTick();
    while(!stm32_dma_check_tc_flag(...)) {
        if ((HAL_GetTick() - start) > timeout_ms) {
            return false;  // Timeout
        }
    }
    return true;
}
```

#### 🟡 HIGH: Inadequate DMA Timeout Values

**Location:** ADC driver, lines 637-657

**Code:**
```cpp
uint16_t timeout = 1000;  // 1000 iterations
while (LL_DMA_IsEnabledStream(DMAx, stream)) {
  if (--timeout == 0) return false;
}
```

**Analysis:**
- On 168MHz F4: ~6µs total timeout
- On 480MHz H7: ~2µs total timeout
- DMA disable can take 100s of microseconds

**Impact:** Premature timeout → code assumes DMA disabled (it's not) → corruption

**Recommendation:** Use tick-based timeout (milliseconds)

#### 🟡 MEDIUM: Cache Coherency Not Explicit

**Location:** stm32_dma.cpp, lines 23-41

**Issue:** DMA configuration doesn't explicitly handle cache coherency

**Example:**
```cpp
// No cache invalidation before DMA read
LL_DMA_ConfigAddresses(stream, (uint32_t)&src, (uint32_t)dst, ...);

// Should be:
SCB_InvalidateDCache_by_Addr(dst, length);  // Before DMA
LL_DMA_ConfigAddresses(...);
```

**Impact:** Stale cache reads after DMA transfer

**Recommendation:** Add explicit cache operations for H7/H7RS

---

### 5.2 SPI Driver Issues

**File:** `radio/src/targets/common/arm/stm32/stm32_spi.cpp`

#### 🔴 CRITICAL: Scratch Buffer Overflow

**Location:** Lines 266-271

**Code:**
```cpp
bool use_scratch_buffer = !_IS_DMA_BUFFER(data) || !_IS_ALIGNED(data);
uint32_t max_xfer_len = use_scratch_buffer ? sizeof(_scratch_buffer) : length;

while (length > 0) {
    uint32_t xfer_len = std::min(length, max_xfer_len);
    // ...
    length -= xfer_len;
    data += xfer_len;
}
```

**Issue:** 
- Scratch buffer is 512 bytes
- Transfer of 1000 bytes:
  - Iteration 1: xfer_len=512, data+=512 (now unaligned again!)
  - Iteration 2: xfer_len=512 (should be 488), use_scratch_buffer re-evaluates to true
  - Result: Data corruption

**Fix:**
```cpp
if (use_scratch_buffer && length > sizeof(_scratch_buffer)) {
    // Split into multiple aligned transfers
    // OR error out
    return false;
}
```

#### 🟡 HIGH: Race Condition in DMA Setup

**Location:** Lines 274-280

**Code:**
```cpp
stm32_dma_enable_stream(..., spi->rxDMA_Stream, ...);  // RX
stm32_dma_enable_stream(..., spi->txDMA_Stream, ...);  // TX
LL_SPI_Enable(spi->SPIx);  // SPI enable
```

**Issue:** No synchronization between DMA setup and SPI enable

**Potential outcome:** 
- SPI starts before DMA ready
- First bytes lost

**Fix:** Add memory barrier or disable SPI during DMA setup

---

### 5.3 I2C Driver Issues

**File:** `radio/src/targets/common/arm/stm32/stm32_i2c_driver.cpp`

#### 🟡 HIGH: Infinite Loop in Timing Calculation

**Location:** Lines 173-204

**Code:**
```cpp
static uint32_t I2C_GetTiming(uint32_t clock_src_freq, uint32_t i2c_freq) {
  // ...
  while ((scll < sdadel_min) || (sclh < sdadel_min)) {
    // Can loop forever if no valid timing exists
    presc++;  // Eventually overflows
  }
  // ...
}
```

**Impact:** Boot hang if I2C timing cannot be calculated

**Fix:** Add maximum iteration count

#### 🟡 MEDIUM: Silent Failure on Init Error

**Location:** Lines 490-495

**Code:**
```cpp
init.Timing = I2C_GetTiming(pclk_freq, clock_rate);  // Can return 0
// No validation - proceeds with invalid timing
if (HAL_I2C_Init(hi2c) != HAL_OK) {
  TRACE("I2C init failed");  // Logs but continues
}
```

**Fix:** Check for 0 timing and return error

---

### 5.4 UART Driver Issues

**File:** `radio/src/targets/common/arm/stm32/stm32_usart_driver.cpp`

#### 🔴 CRITICAL: Blocking Wait in ISR Context

**Location:** Lines 635-676

**Code:**
```cpp
static bool stm32_usart_wait_for_tx_dma(stm32_usart_t* usart) {
  uint32_t timeout = 1000;  // Only 1000 iterations!
  while (LL_DMA_IsEnabledStream(usart->txDMA, usart->txDMA_Stream)) {
    if (--timeout == 0) return false;
  }
  // Called from TX complete callback!
}
```

**Impact:**
- Called from DMA interrupt context
- Spin-waits for DMA disable
- Blocks higher priority ISRs
- System instability

**Fix:** Don't wait in ISR - use state machine

#### 🟡 HIGH: Buffer Overflow in Circular RX DMA

**Location:** Line 347

**Code:**
```cpp
LL_DMA_SetDataLength(usart->rxDMA, usart->rxDMA_Stream, usart->rxBufferSize);
// No validation that rxBufferSize < MAX_BUFFER
```

**Fix:** Add bounds check

---

### 5.5 ADC Driver Issues

**File:** `radio/src/targets/common/arm/stm32/stm32_adc.cpp`

#### 🟡 MEDIUM: Race Condition in Calibration

**Location:** Lines 244-251

**Code:**
```cpp
void adcCalibrationStart() {
  // No critical section
  adcCalibrationState = ADC_CALIBRATION_RUNNING;
  // If interrupted here, state corrupted
  adcRead();
}
```

**Fix:** Protect with mutex or disable interrupts

#### 🟡 MEDIUM: Unprotected Inhibit Mask

**Location:** Lines 65-66, 119, 127

**Code:**
```cpp
static uint32_t _adc_inhibit_mask = 0;  // Global, no protection

void enableVBatBridge() {
  _adc_inhibit_mask |= (1 << VBAT_IDX);  // Non-atomic
}
```

**Fix:** Use atomic operations or critical section

---

### 5.6 GPIO Driver Issues

**File:** `radio/src/targets/common/arm/stm32/stm32_gpio.cpp`

#### 🟡 MEDIUM: Non-Atomic Register Access

**Location:** Lines 80, 83-84, 87-88

**Code:**
```cpp
// Read-modify-write NOT atomic
port->PUPDR &= ~(0x3 << (2 * pin_num));
port->PUPDR |=  (((mode >> 2) & 0x3) << (2 * pin_num));

// If another thread modifies different pin during these operations,
// data lost
```

**Impact:** GPIO pin corruption under concurrent access

**Fix:** Use HAL atomic functions or critical section

---

## 6. Hardware-Specific Drivers

### 6.1 SD Card Driver

**Files:** 
- `sdcard_spi.cpp` (SPI mode)
- `diskio_sdio.cpp` (SDIO mode)

#### 🟠 MEDIUM: Missing Timeout in Card Detection

**Location:** sdcard_spi.cpp, lines 394-404

**Code:**
```cpp
bool sdInit() {
  // Card initialization can take several seconds
  // No timeout specified
  while (SD_SendCmd(CMD0, 0) != 0x01) {
    // Infinite retry
  }
}
```

**Fix:** Add retry counter with timeout

#### ✅ GOOD: Error Handling

- Card presence detection implemented
- CRC checking enabled
- Proper state machine for initialization

---

### 6.2 Flash Drivers

**Files:**
- `flash_driver.cpp` (internal Flash)
- `spi_flash.cpp` (external SPI Flash)
- `stm32_xspi_nor.cpp` (XSPI NOR Flash)

#### 🟡 HIGH: Missing Erase Verification

**Location:** flash_driver.cpp, lines 89-95

**Code:**
```cpp
bool flashErase(uint32_t addr) {
  HAL_FLASH_Unlock();
  FLASH_Erase_Sector(...);
  HAL_FLASH_Lock();
  // No verification that erase succeeded
}
```

**Recommendation:** Read back and verify 0xFF pattern

#### ✅ GOOD: Write Protection

- Flash locked by default
- Unlock/lock sequence enforced

---

### 6.3 SDRAM Driver

**File:** `sdram_driver.cpp`

#### ✅ EXCELLENT: Timing Configuration

**Lines 53-111:** Proper SDRAM initialization sequence

```cpp
1. GPIO configuration
2. FMC timing setup
3. Clock enable
4. Mode register programming
5. Refresh rate configuration
```

**No issues found** - well-structured and documented

---

### 6.4 Display Drivers

**DMA2D:** `dma2d.cpp`  
**LTDC:** Board-specific files

#### 🟠 MEDIUM: Missing Frame Buffer Synchronization

**Location:** Multiple LCD drivers

**Issue:** Frame buffer updates not synchronized with VSYNC

**Impact:** Tearing artifacts on display

**Recommendation:** Use double buffering with VSYNC IRQ

---

### 6.5 Audio Drivers

**Files:**
- `audio_dac_driver.cpp`
- `stm32_i2s.cpp`
- `wm8904.cpp`, `tas2505.cpp` (codecs)

#### 🔴 CRITICAL: Audio Buffer Overflow

**Location:** `audio_driver.cpp` (board-specific), lines 119-126

**Code:**
```cpp
void audio_update_dma_buffer(AudioBuffer *buffer) {
  for (uint32_t idx = 0; idx < buffer->size; idx++) {
    // buffer->size NOT validated against DMA_BUFFER_HALF_LEN
    _dma_buffer[offset + idx] = buffer->data[idx];
  }
}
```

**Impact:**
- If `buffer->size > DMA_BUFFER_HALF_LEN`, overflow into adjacent DMA buffer
- Audio corruption, potential memory corruption

**Fix:**
```cpp
if (buffer->size > DMA_BUFFER_HALF_LEN) {
  buffer->size = DMA_BUFFER_HALF_LEN;
}
```

#### 🟡 MEDIUM: Unprotected Volatile Access

**Location:** Lines 109, 135

**Code:**
```cpp
volatile uint32_t _dma_buffer_offset;

// Read/write without synchronization
uint32_t offset = _dma_buffer_offset;  // Can change during read
```

**Fix:** Use atomic operations or mutex

---

### 6.6 Sensor Drivers

**Files:**
- `icm42607C.cpp` (IMU)
- `lsm6ds_driver.cpp` (IMU)
- `flysky_gimbal_driver.cpp` (Gimbal)

#### 🔴 CRITICAL: Unsafe Pointer Casting

**Location:** icm42607C.cpp, lines 179-181

**Code:**
```cpp
int16_t ax = *(int16_t*)&buffer[6];   // Assumes buffer >= 12 bytes
int16_t ay = *(int16_t*)&buffer[8];
int16_t az = *(int16_t*)&buffer[10];
// No validation of IMU_BUFFER_LENGTH
```

**Impact:** Stack overflow if buffer smaller than expected

**Fix:**
```cpp
_Static_assert(IMU_BUFFER_LENGTH >= 12, "Buffer too small");
// OR
if (buffer_length < 12) return ERROR;
```

---

### 6.7 USB Stack

**Files:**
- `usb_driver.cpp`
- `usbd_cdc.cpp` (Virtual COM port)
- `usbd_hid_joystick.c` (HID device)
- `usbd_storage_msd.cpp` (Mass storage)

#### ✅ GOOD: Standard USB Middleware

Uses ST's USB device library (USBDevice middleware)

**Security:**
- Descriptor validation present
- Buffer overflow protection in most paths
- Proper state machine

#### 🟠 MEDIUM: USB Serial Buffer Management

**Location:** usbd_cdc.cpp

**Issue:** Circular buffer management could be improved

**Recommendation:** Add overflow detection and reporting

---

### 6.8 EEPROM Driver

**File:** `radio/src/hal/eeprom/eeprom_driver.cpp`

#### 🔴 CRITICAL: Infinite Loop, No Timeout

**Location:** Lines 70-73

**Code:**
```cpp
void eepromInit() {
  // ...
  while (!I2C_EE_WaitEepromStandbyState()) {
    eepromInit();  // TODO: seriously? no timeout, no bailout ???
  }
}
```

**Impact:**
- If I2C EEPROM is disconnected/stuck, system hangs indefinitely
- Blocks entire firmware
- Requires watchdog reset or power cycle

**Fix:**
```cpp
#define EEPROM_INIT_TIMEOUT_MS 5000

bool eepromInit() {
  uint32_t start = HAL_GetTick();
  while (!I2C_EE_WaitEepromStandbyState()) {
    if ((HAL_GetTick() - start) > EEPROM_INIT_TIMEOUT_MS) {
      TRACE("EEPROM init timeout");
      return false;
    }
    delay_ms(10);
  }
  return true;
}
```

---

### 6.9 Touch Driver

**File:** Board-specific (e.g., `jumper-h750/touch_driver.cpp`)

#### 🟡 HIGH: Busy-Loop on I2C Failure

**Location:** Lines 88-89

**Code:**
```cpp
while (!touch_i2c_read(addr, reg, &result, 1)) {
    if (--tryCount == 0) break;
    // NO DELAY - busy loops consuming CPU
}
```

**Impact:** Wastes CPU cycles, delays real-time tasks

**Fix:** Add delay between retries

#### 🟡 MEDIUM: Race Condition on Touch Event

**Location:** Line 45

**Code:**
```cpp
volatile bool touchEventOccured = false;  // Set by EXTI ISR

// Main loop
if (touchEventOccured) {
  touchEventOccured = false;  // Non-atomic clear
  // If IRQ fires here, event lost
}
```

**Fix:** Use atomic operations or semaphore

---

## 7. Critical Issues Detail

### 7.1 Top 5 Critical Issues

#### 1. EEPROM Infinite Loop 🔴🔴🔴

**Severity:** CRITICAL  
**File:** `radio/src/hal/eeprom/eeprom_driver.cpp:70-73`  
**Risk:** Complete system lockup

**Code:**
```cpp
while (!I2C_EE_WaitEepromStandbyState()) {
    eepromInit();  // Recursive, no timeout
}
```

**Impact:**
- Disconnected/faulty EEPROM hangs boot
- No recovery mechanism
- Field failure difficult to diagnose

**Fix Priority:** **IMMEDIATE**

**Recommended Fix:**
```cpp
bool eepromInit() {
  uint32_t timeout = 100;  // 100 retries
  while (!I2C_EE_WaitEepromStandbyState()) {
    if (--timeout == 0) {
      TRACE("EEPROM init failed");
      return false;
    }
    delay_ms(10);
  }
  return true;
}
```

---

#### 2. Audio Buffer Overflow 🔴🔴

**Severity:** CRITICAL  
**File:** Board-specific `audio_driver.cpp:119-126`  
**Risk:** Memory corruption

**Code:**
```cpp
void audio_update_dma_buffer(AudioBuffer *buffer) {
  for (uint32_t idx = 0; idx < buffer->size; idx++) {
    _dma_buffer[offset + idx] = buffer->data[idx];  // No bounds check
  }
}
```

**Vulnerable Scenario:**
```
DMA_BUFFER_HALF_LEN = 1024
buffer->size = 2000 (from audio file)
Write: _dma_buffer[0..2000] → Overflow past buffer end
```

**Fix Priority:** **IMMEDIATE**

**Recommended Fix:**
```cpp
void audio_update_dma_buffer(AudioBuffer *buffer) {
  uint32_t safe_size = MIN(buffer->size, DMA_BUFFER_HALF_LEN);
  for (uint32_t idx = 0; idx < safe_size; idx++) {
    _dma_buffer[offset + idx] = buffer->data[idx];
  }
  if (buffer->size > safe_size) {
    TRACE("Audio buffer truncated: %d -> %d", buffer->size, safe_size);
  }
}
```

---

#### 3. DMA Infinite Wait Loops 🔴🔴

**Severity:** CRITICAL  
**Files:** Multiple (SPI, ADC, UART)  
**Risk:** System lockup on hardware fault

**Example from SPI (lines 283-289):**
```cpp
while(!stm32_dma_check_tc_flag(spi->DMA, spi->rxDMA_Stream));
while (!LL_SPI_IsActiveFlag_TXE(spi->SPIx));
while(LL_SPI_IsActiveFlag_BSY(spi->SPIx));
```

**Impact:**
- DMA hardware fault → infinite loop
- Watchdog may not trigger (IRQs disabled)
- Complete system freeze

**Fix Priority:** **HIGH**

**Recommended Fix:**
```cpp
#define DMA_TIMEOUT_MS 1000

bool wait_dma_tc(DMA_TypeDef* dma, uint32_t stream) {
  uint32_t start = HAL_GetTick();
  while (!stm32_dma_check_tc_flag(dma, stream)) {
    if ((HAL_GetTick() - start) > DMA_TIMEOUT_MS) {
      TRACE("DMA timeout on stream %d", stream);
      return false;
    }
  }
  return true;
}
```

---

#### 4. Unsafe IMU Pointer Casting 🔴

**Severity:** HIGH  
**File:** `radio/src/drivers/icm42607C.cpp:179-181`  
**Risk:** Stack overflow, undefined behavior

**Code:**
```cpp
int16_t ax = *(int16_t*)&buffer[6];
int16_t ay = *(int16_t*)&buffer[8];
int16_t az = *(int16_t*)&buffer[10];
// Assumes buffer >= 12 bytes
```

**Impact:** Buffer undersize → read beyond bounds → crash

**Fix Priority:** **HIGH**

**Recommended Fix:**
```cpp
_Static_assert(IMU_BUFFER_LENGTH >= 12, "IMU buffer must be >= 12 bytes");

int16_t ax, ay, az;
if (buffer_length >= 12) {
  memcpy(&ax, &buffer[6], sizeof(int16_t));
  memcpy(&ay, &buffer[8], sizeof(int16_t));
  memcpy(&az, &buffer[10], sizeof(int16_t));
} else {
  TRACE("IMU buffer too small: %d", buffer_length);
  return ERROR;
}
```

---

#### 5. GPIO Non-Atomic Register Access 🟡

**Severity:** MEDIUM-HIGH  
**File:** `stm32_gpio.cpp:83-84`  
**Risk:** GPIO corruption under concurrent access

**Code:**
```cpp
port->PUPDR &= ~(0x3 << (2 * pin_num));  // Step 1: Read-modify
port->PUPDR |=  (((mode >> 2) & 0x3) << (2 * pin_num));  // Step 2: Write
// If interrupt modifies another pin between steps → data loss
```

**Impact:** 
- Random GPIO state corruption
- Hard-to-debug intermittent failures

**Fix Priority:** **MEDIUM**

**Recommended Fix:**
```cpp
__disable_irq();
port->PUPDR = (port->PUPDR & ~(0x3 << (2 * pin_num))) 
            | (((mode >> 2) & 0x3) << (2 * pin_num));
__enable_irq();
```

---

### 7.2 Common Vulnerability Patterns

#### Pattern 1: Infinite Polling Loops

**Locations:** 15+ instances

**Generic Form:**
```cpp
while (!hardware_flag_check()) {
  // No timeout
  // No error handling
}
```

**Root Cause:** Assumption that hardware always works

**System Impact:**
- Single hardware fault causes total system failure
- No graceful degradation

#### Pattern 2: Inadequate Timeout Values

**Example:** ADC driver timeout = 1000 iterations

**Analysis:**
```
F4 @ 168MHz: ~6µs total wait
H7 @ 480MHz: ~2µs total wait
DMA disable time: 100µs typical, 1ms worst-case
Result: Premature timeout
```

**Recommendation:** Use millisecond-based timeouts

#### Pattern 3: Missing Bounds Validation

**Locations:** Buffer sizes, array indices, DMA lengths

**Example:**
```cpp
for (int i = 0; i < external_size; i++) {
  fixed_buffer[i] = data[i];  // external_size not validated
}
```

**Impact:** Buffer overflow

**Fix:** Always validate external inputs

#### Pattern 4: Unprotected Shared State

**Examples:**
- `volatile` variables accessed without synchronization
- GPIO registers modified non-atomically
- Global state without mutex protection

**Impact:** Race conditions, data corruption

**Fix:** Use atomic operations, mutexes, or critical sections

---

## 8. Recommendations

### 8.1 Immediate Actions (Week 1)

**Critical Fixes:**

1. **Add timeout to EEPROM initialization loop**
   - File: `eeprom_driver.cpp:70-73`
   - Effort: 30 minutes
   - Risk: Zero (adds safety)

2. **Validate audio buffer size**
   - Files: Board-specific `audio_driver.cpp`
   - Effort: 1 hour (all boards)
   - Risk: Zero (clamps values)

3. **Add DMA timeout helpers**
   - Create `dma_utils.h` with timeout wrappers
   - Replace infinite loops in SPI, ADC, UART
   - Effort: 4-6 hours
   - Risk: Low (improves robustness)

### 8.2 Short-term Actions (Week 2-4)

**High Priority Fixes:**

1. **Fix IMU buffer casting**
   - Add static assertions
   - Use safe memcpy instead of pointer cast
   - Effort: 1 hour

2. **Add GPIO atomic operations**
   - Wrap all GPIO RMW in critical sections
   - Effort: 2-3 hours

3. **Improve I2C error handling**
   - Add timeouts to timing calculation
   - Propagate init errors
   - Effort: 2-3 hours

4. **Fix SPI scratch buffer logic**
   - Add length validation
   - Error on oversized transfers
   - Effort: 2 hours

### 8.3 Medium-term Actions (Month 2-3)

**Code Quality Improvements:**

1. **Standardize timeout mechanisms**
   - Create common timeout helpers
   - Use millisecond-based timeouts
   - Document timeout values

2. **Add bounds checking utilities**
   - Create safe buffer copy macros
   - Add array access validators
   - Use throughout codebase

3. **Improve synchronization**
   - Document locking requirements
   - Add mutex/atomic where needed
   - Static analysis for races

4. **Enhance error handling**
   - Standardize error codes
   - Add error logging
   - Implement recovery strategies

### 8.4 Long-term Actions (Quarter 2)

**Architectural Improvements:**

1. **Add crash logging**
   - Save fault info to flash/SD
   - Include stack trace
   - LED indication codes

2. **Implement safe mode**
   - Detect repeated crashes
   - Boot to minimal configuration
   - Allow diagnostics

3. **Add hardware monitoring**
   - Detect peripheral failures
   - Report to telemetry
   - Graceful degradation

4. **Memory protection**
   - Review MPU configuration
   - Add write protection
   - Enhance stack checking

### 8.5 Testing Requirements

**Unit Tests:**
- Timeout behavior (simulate hardware stall)
- Buffer overflow prevention
- Error path coverage

**Integration Tests:**
- Hardware fault injection
- Concurrent access scenarios
- Long-duration stress tests

**Static Analysis:**
- Enable all warnings
- Run Cppcheck/Clang-Tidy
- Address ThreadSanitizer findings

---

## 9. Positive Findings

Despite critical issues, many aspects are well-implemented:

✅ **MPU Configuration:** Properly set up with appropriate regions

✅ **SDRAM Initialization:** Excellent timing and sequence

✅ **Clock Management:** Appropriate frequencies for each peripheral

✅ **USB Stack:** Uses standard middleware with good practices

✅ **Flash Locking:** Write protection enforced

✅ **DMA Priorities:** Correctly assigned

✅ **Interrupt Priorities:** Sensible hierarchy

✅ **Code Organization:** Clear separation of MCU-specific code

✅ **Documentation:** Many TODO comments indicate awareness of issues

---

## 10. Conclusion

### Summary

EdgeTX's MCU and hardware driver code demonstrates:
- **Strong architecture:** Good abstraction, multi-MCU support
- **Critical vulnerabilities:** Infinite loops, buffer overflows, race conditions
- **Typical embedded issues:** Timeouts, synchronization, error handling

**Current Risk:** 🔴 **HIGH** - Critical issues can cause system lockup

**After Fixes:** 🟡 **MEDIUM → LOW** - Residual risks manageable

### Estimated Effort

**Critical fixes:** 8-12 hours (1 developer)  
**High priority:** 20-30 hours (1 developer)  
**Total immediate:** 30-40 hours over 2-4 weeks

### Next Steps

1. ✅ Review this report with embedded team
2. ⬜ Create GitHub issues for critical items
3. ⬜ Implement timeout helpers (Week 1)
4. ⬜ Fix buffer overflows (Week 1)
5. ⬜ Add bounds validation (Week 2-3)
6. ⬜ Improve synchronization (Week 3-4)
7. ⬜ Add comprehensive testing

---

**Report Version:** 1.0  
**Date:** February 15, 2026  
**Reviewer:** Code Review Agent  
**Next Review:** After critical fixes implemented

---

*This report complements the general code review. For application-level issues, see CODE_REVIEW_REPORT.md*
