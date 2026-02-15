# EdgeTX MCU & Driver Code Inspection - Quick Reference

**One-page summary of MCU and hardware driver findings**

---

## 📊 Inspection Statistics

| Category | Count |
|----------|-------|
| Files Reviewed | 324 |
| MCU Families | 4 (F2, F4, H7, H7RS) |
| Peripheral Drivers | 20+ |
| Critical Issues | 6 |
| High Priority | 12 |
| Medium Priority | 15+ |

---

## 🔴 Top 5 Critical Issues

| # | Issue | File | Impact | Fix Time |
|---|-------|------|--------|----------|
| 1 | **EEPROM infinite loop** | eeprom_driver.cpp:70 | System lockup | 30 min |
| 2 | **Audio buffer overflow** | audio_driver.cpp:119 | Memory corruption | 1 hour |
| 3 | **DMA infinite waits** | Multiple files | System freeze | 4-6 hours |
| 4 | **IMU unsafe casting** | icm42607C.cpp:179 | Stack overflow | 1 hour |
| 5 | **GPIO race conditions** | stm32_gpio.cpp:83 | Pin corruption | 2 hours |

---

## 🎯 Critical Code Examples

### 1. EEPROM Hang (CRITICAL)

**Problem:**
```cpp
// eeprom_driver.cpp:70-73
while (!I2C_EE_WaitEepromStandbyState()) {
    eepromInit();  // Infinite recursion, no timeout!
}
```

**Fix:**
```cpp
bool eepromInit() {
  uint32_t timeout = 100;
  while (!I2C_EE_WaitEepromStandbyState()) {
    if (--timeout == 0) return false;
    delay_ms(10);
  }
  return true;
}
```

---

### 2. Audio Buffer Overflow (CRITICAL)

**Problem:**
```cpp
// audio_driver.cpp:119-126
for (uint32_t idx = 0; idx < buffer->size; idx++) {
    _dma_buffer[offset + idx] = buffer->data[idx];  // NO BOUNDS CHECK
}
```

**Fix:**
```cpp
uint32_t safe_size = MIN(buffer->size, DMA_BUFFER_HALF_LEN);
for (uint32_t idx = 0; idx < safe_size; idx++) {
    _dma_buffer[offset + idx] = buffer->data[idx];
}
```

---

### 3. DMA Infinite Loop (CRITICAL)

**Problem:**
```cpp
// stm32_spi.cpp:283-289
while(!stm32_dma_check_tc_flag(spi->DMA, spi->rxDMA_Stream));  // NO TIMEOUT
while (!LL_SPI_IsActiveFlag_TXE(spi->SPIx));                    // NO TIMEOUT
```

**Fix:**
```cpp
bool wait_dma_tc(DMA_TypeDef* dma, uint32_t stream) {
  uint32_t start = HAL_GetTick();
  while (!stm32_dma_check_tc_flag(dma, stream)) {
    if ((HAL_GetTick() - start) > 1000) return false;
  }
  return true;
}
```

---

### 4. IMU Unsafe Casting (HIGH)

**Problem:**
```cpp
// icm42607C.cpp:179-181
int16_t ax = *(int16_t*)&buffer[6];   // Assumes buffer >= 12 bytes
int16_t ay = *(int16_t*)&buffer[8];
int16_t az = *(int16_t*)&buffer[10];
```

**Fix:**
```cpp
_Static_assert(IMU_BUFFER_LENGTH >= 12, "Buffer too small");
memcpy(&ax, &buffer[6], sizeof(int16_t));
memcpy(&ay, &buffer[8], sizeof(int16_t));
memcpy(&az, &buffer[10], sizeof(int16_t));
```

---

### 5. GPIO Race Condition (MEDIUM)

**Problem:**
```cpp
// stm32_gpio.cpp:83-84
port->PUPDR &= ~(0x3 << (2 * pin_num));  // Read
port->PUPDR |=  (((mode >> 2) & 0x3) << (2 * pin_num));  // Modify-Write
// If interrupted between lines, data lost
```

**Fix:**
```cpp
__disable_irq();
port->PUPDR = (port->PUPDR & ~(0x3 << (2 * pin_num))) 
            | (((mode >> 2) & 0x3) << (2 * pin_num));
__enable_irq();
```

---

## 📅 Fix Timeline

```
Week 1 (Critical):
  Day 1: EEPROM timeout         (30 min)
  Day 2: Audio bounds check     (1 hour)
  Day 3-5: DMA timeout wrappers (6 hours)
  
Week 2 (High):
  Day 1: IMU safe casting       (1 hour)
  Day 2: GPIO atomicity         (2 hours)
  Day 3: I2C error handling     (2 hours)
  Day 4: SPI scratch buffer     (2 hours)
  
Week 3-4 (Medium):
  - Remaining race conditions
  - Cache coherency
  - Error propagation
```

**Total Effort:** 30-40 hours (1 developer)

---

## 🏗️ MCU Architecture Overview

### Supported Platforms

| MCU | Core | RAM | Flash | Radios |
|-----|------|-----|-------|--------|
| F2 | M3 | 128KB | 512KB | Old Taranis |
| F4 | M4 | 256KB | 2MB | X9D+, X10 |
| H7 | M7 | 1MB | 2MB | Horus, TX16S |
| H7RS | M7+ | 640KB | 2MB | Future |

### Memory Layout (H7 Example)

```
0x00000000  ITCMRAM (64KB)    - Fast instruction
0x20000000  DTCMRAM (128KB)   - ISR vectors
0x24000000  RAM_D1 (512KB)    - BSS
0xC0000000  SDRAM (8MB)       - Code + Heap
0x90000000  NORFLASH (8MB)    - Storage
```

---

## 🔧 Common Vulnerability Patterns

### Pattern 1: Infinite Polling
```cpp
while (!flag);  // ❌ NO TIMEOUT
```
**Fix:** Add timeout with HAL_GetTick()

### Pattern 2: Inadequate Timeout
```cpp
timeout = 1000;  // ❌ Only microseconds on fast MCU
```
**Fix:** Use millisecond-based timeouts

### Pattern 3: Missing Bounds Check
```cpp
for (i = 0; i < ext_size; i++)
  buf[i] = data[i];  // ❌ ext_size not validated
```
**Fix:** Validate ext_size <= BUF_SIZE

### Pattern 4: Unprotected Shared State
```cpp
volatile uint32_t shared;
shared = value;  // ❌ Non-atomic
```
**Fix:** Use __disable_irq() or atomic ops

---

## ✅ Positive Findings

- ✅ MPU properly configured
- ✅ SDRAM initialization excellent
- ✅ Clock management appropriate
- ✅ USB stack uses standard middleware
- ✅ Flash write protection enforced
- ✅ Interrupt priorities sensible

---

## 📋 Action Checklist

### Immediate (This Week)
- [ ] Add EEPROM timeout
- [ ] Fix audio buffer overflow
- [ ] Create DMA timeout helpers

### Short-term (Next 2 Weeks)
- [ ] Fix IMU buffer casting
- [ ] Add GPIO atomic operations
- [ ] Improve I2C error handling
- [ ] Fix SPI scratch buffer

### Medium-term (Next Month)
- [ ] Standardize timeout mechanisms
- [ ] Add bounds checking utilities
- [ ] Improve synchronization
- [ ] Enhance error handling

### Long-term (Next Quarter)
- [ ] Add crash logging
- [ ] Implement safe mode
- [ ] Add hardware monitoring
- [ ] Memory protection review

---

## 🔬 Testing Requirements

**Unit Tests:**
- Timeout behavior (simulate stall)
- Buffer overflow prevention
- Error path coverage

**Integration Tests:**
- Hardware fault injection
- Concurrent access scenarios
- Long-duration stress tests

**Static Analysis:**
- Cppcheck/Clang-Tidy
- ThreadSanitizer
- AddressSanitizer

---

## 📊 Risk Assessment

| State | Level | Description |
|-------|-------|-------------|
| **Current** | 🔴 HIGH | Infinite loops can lock system |
| **After Week 1** | 🟡 MEDIUM | Critical issues fixed |
| **After Week 4** | 🟢 LOW | Most issues resolved |

---

## 📞 Key Files Reference

**Critical Files:**
- `eeprom_driver.cpp` - EEPROM with infinite loop
- `audio_driver.cpp` - DMA buffer overflow
- `stm32_spi.cpp` - DMA infinite waits
- `stm32_adc.cpp` - Inadequate timeouts
- `stm32_gpio.cpp` - Race conditions
- `icm42607C.cpp` - Unsafe pointer casts

**Architecture Files:**
- `system_init.c` - Boot sequence & MPU
- `cortex_m_isr.c` - Interrupt handlers
- `stm32_*_hal_conf.h` - Peripheral config
- `layout.ld` - Memory layout

---

## 📚 Related Documents

- **[MCU_DRIVER_INSPECTION_REPORT.md](MCU_DRIVER_INSPECTION_REPORT.md)** - Complete detailed report
- **[CODE_REVIEW_REPORT.md](CODE_REVIEW_REPORT.md)** - General code review
- **[SECURITY_FIXES.md](SECURITY_FIXES.md)** - Application-level fixes

---

**Last Updated:** February 15, 2026  
**Report Version:** 1.0  
**Next Review:** After critical fixes

---

*For complete analysis with line numbers and detailed explanations, see MCU_DRIVER_INSPECTION_REPORT.md*
