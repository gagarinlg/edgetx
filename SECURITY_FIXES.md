# EdgeTX Code Review - Top Priority Fixes

**Critical Security Vulnerabilities with Code Examples and Fixes**

This document provides specific code examples of critical issues found and recommended fixes.

---

## 1. BUFFER OVERFLOW VULNERABILITIES 🔴 CRITICAL

### Issue 1.1: Unsafe strcpy in bluetooth.cpp

**File:** `radio/src/bluetooth.cpp`  
**Line:** 533

**Current Code (UNSAFE):**
```cpp
strcpy(distantAddr, &line[10]); // TODO quick & dirty
```

**Problem:** No bounds checking. If `line` is longer than `distantAddr` buffer, overflow occurs.

**Recommended Fix:**
```cpp
// Option 1: Using strncpy
strncpy(distantAddr, &line[10], sizeof(distantAddr) - 1);
distantAddr[sizeof(distantAddr) - 1] = '\0';

// Option 2: Using snprintf (better for visibility)
snprintf(distantAddr, sizeof(distantAddr), "%s", &line[10]);

// Option 3: Using safe wrapper (create once, use everywhere)
safe_string_copy(distantAddr, &line[10], sizeof(distantAddr));
```

---

### Issue 1.2: Multiple unsafe operations in model_audio.cpp

**File:** `radio/src/model_audio.cpp`  
**Lines:** 31, 90-91

**Current Code (UNSAFE):**
```cpp
// Line 31
strcpy(path, SOUNDS_PATH "/");

// Lines 90-91
strcpy(str, _suffixes[event]);
strcat(str, SOUNDS_EXT);
```

**Problem:** No validation that buffers are large enough.

**Recommended Fix:**
```cpp
// Line 31
snprintf(path, PATH_MAX, "%s/", SOUNDS_PATH);

// Lines 90-91
snprintf(str, MAX_STR_LEN, "%s%s", _suffixes[event], SOUNDS_EXT);
```

---

### Issue 1.3: sprintf without bounds in radiodata.cpp

**File:** `companion/src/firmwares/radiodata.cpp`

**Current Code (UNSAFE):**
```cpp
sprintf(model.filename, "model%d.%s", index + 1, hasSDCard ? "yml" : "bin");
sprintf(filename, "model%d.yml", ++index);
```

**Problem:** If `index` is very large, could overflow buffer.

**Recommended Fix:**
```cpp
snprintf(model.filename, sizeof(model.filename), 
         "model%d.%s", index + 1, hasSDCard ? "yml" : "bin");
snprintf(filename, sizeof(filename), "model%d.yml", ++index);
```

---

### Issue 1.4: Unsafe strcpy in mdichild.cpp

**File:** `companion/src/mdichild.cpp`

**Current Code (UNSAFE):**
```cpp
strcpy(radioData.models[modelIdx].filename, 
       radioData.getNextModelFilename().toStdString().c_str());
```

**Problem:** 
1. No bounds checking
2. QString to C-string conversion could fail
3. Temporary string could be destroyed

**Recommended Fix:**
```cpp
std::string modelFilename = radioData.getNextModelFilename().toStdString();
if (modelFilename.length() < sizeof(radioData.models[modelIdx].filename)) {
    strncpy(radioData.models[modelIdx].filename, 
            modelFilename.c_str(),
            sizeof(radioData.models[modelIdx].filename) - 1);
    radioData.models[modelIdx].filename[
        sizeof(radioData.models[modelIdx].filename) - 1] = '\0';
} else {
    // Handle error - filename too long
    TRACE("Model filename too long: %s", modelFilename.c_str());
}
```

---

## 2. RESOURCE LEAKS 🔴 CRITICAL

### Issue 2.1: Unchecked file open in simudisk.cpp

**File:** `radio/src/targets/simu/simudisk.cpp`  
**Line:** 41

**Current Code (UNSAFE):**
```cpp
disk_image = fopen(...);
// No error check - disk_image could be NULL
```

**Recommended Fix:**
```cpp
disk_image = fopen(filename, "rb+");
if (disk_image == NULL) {
    TRACE("Failed to open disk image: %s", filename);
    return ERROR_CODE;
}
// ... use disk_image ...
if (disk_image) {
    fclose(disk_image);
    disk_image = NULL;
}
```

**Better Fix (C++):**
```cpp
class DiskImageFile {
    FILE* file;
public:
    DiskImageFile(const char* filename) {
        file = fopen(filename, "rb+");
        if (!file) throw std::runtime_error("Failed to open disk");
    }
    ~DiskImageFile() {
        if (file) fclose(file);
    }
    FILE* get() { return file; }
};

// Usage
try {
    DiskImageFile disk(filename);
    // Use disk.get()
    // Automatically closed on scope exit
} catch (const std::exception& e) {
    TRACE("Error: %s", e.what());
}
```

---

### Issue 2.2: Unchecked QFile open in modelprinter.cpp

**File:** `companion/src/print/modelprinter.cpp`  
**Lines:** 28-29

**Current Code (UNSAFE):**
```cpp
QFile file("foo.html");
file.open(QIODevice::Truncate | QIODevice::WriteOnly);
// No check if open succeeded
```

**Recommended Fix:**
```cpp
QFile file("foo.html");
if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
    qWarning() << "Failed to open file:" << file.errorString();
    return false; // or handle error appropriately
}
// ... use file ...
file.close();
```

---

## 3. INPUT VALIDATION 🔴 HIGH

### Issue 3.1: Zip path traversal vulnerability

**File:** `companion/src/storage/minizinterface.cpp`

**Current Code (UNSAFE):**
```cpp
if (!mz_zip_reader_extract_to_file(&zip_archive, i, 
    qPrintable(destPath), 0)) {
    // ...
}
```

**Problem:** No validation of `destPath` - could extract to `../../etc/passwd`

**Recommended Fix:**
```cpp
// Validate path before extraction
QString canonicalDest = QFileInfo(destPath).canonicalFilePath();
QString canonicalBase = QFileInfo(extractionDir).canonicalFilePath();

if (!canonicalDest.startsWith(canonicalBase)) {
    qWarning() << "Path traversal attempt detected:" << destPath;
    continue; // Skip this file
}

if (!mz_zip_reader_extract_to_file(&zip_archive, i, 
    qPrintable(destPath), 0)) {
    // ...
}
```

---

### Issue 3.2: Array access without bounds check

**File:** `radio/src/model_audio.cpp`  
**Line:** 59-64

**Current Code (PARTIALLY SAFE):**
```cpp
if (index <= SWSRC_LAST_SWITCH) {
    div_t swinfo = switchInfo(index);
    auto sw_name = switchGetDefaultName(swinfo.quot);
    if (!sw_name) return false;  // Good!
    str = strAppend(str, sw_name);
    str = strAppend(str, _sw_positions[swinfo.rem]);  // Is swinfo.rem valid?
}
```

**Problem:** `swinfo.rem` used as array index without checking bounds of `_sw_positions`

**Recommended Fix:**
```cpp
if (index <= SWSRC_LAST_SWITCH) {
    div_t swinfo = switchInfo(index);
    auto sw_name = switchGetDefaultName(swinfo.quot);
    if (!sw_name) return false;
    
    // Validate array index
    if (swinfo.rem < 0 || swinfo.rem >= 3) {
        TRACE("Invalid switch position: %d", swinfo.rem);
        return false;
    }
    
    str = strAppend(str, sw_name);
    str = strAppend(str, _sw_positions[swinfo.rem]);
}
```

---

## 4. SAFE WRAPPER FUNCTIONS 🟢 RECOMMENDATION

Create a header file `safe_strings.h`:

```cpp
#ifndef SAFE_STRINGS_H
#define SAFE_STRINGS_H

#include <cstring>
#include <cstdio>

// Safe string copy - returns true if entire string was copied
inline bool safe_string_copy(char* dst, const char* src, size_t dst_size) {
    if (!dst || !src || dst_size == 0) return false;
    
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
    
    return strlen(src) < dst_size;
}

// Safe string concatenation - returns true if entire string was appended
inline bool safe_string_append(char* dst, const char* src, size_t dst_size) {
    if (!dst || !src || dst_size == 0) return false;
    
    size_t dst_len = strnlen(dst, dst_size);
    if (dst_len >= dst_size - 1) return false;
    
    strncat(dst, src, dst_size - dst_len - 1);
    return (dst_len + strlen(src)) < dst_size;
}

// Safe printf to buffer
template<typename... Args>
inline bool safe_snprintf(char* buf, size_t buf_size, const char* format, Args... args) {
    if (!buf || buf_size == 0) return false;
    
    int written = snprintf(buf, buf_size, format, args...);
    return written >= 0 && static_cast<size_t>(written) < buf_size;
}

#endif // SAFE_STRINGS_H
```

**Usage Example:**
```cpp
#include "safe_strings.h"

char path[256];
if (!safe_string_copy(path, SOUNDS_PATH "/", sizeof(path))) {
    TRACE("Path too long");
    return ERROR_PATH_TOO_LONG;
}

if (!safe_string_append(path, filename, sizeof(path))) {
    TRACE("Filename too long");
    return ERROR_PATH_TOO_LONG;
}
```

---

## 5. GLOBAL STATE SYNCHRONIZATION 🟠 HIGH

### Issue 5.1: Unsynchronized global access

**Problem:** `g_model` and `g_eeGeneral` accessed from multiple tasks

**Current Pattern (potentially unsafe):**
```cpp
// In multiple files
extern RadioData g_eeGeneral;
extern ModelData g_model;

void someFunction() {
    // Direct access - no lock visible
    if (g_model.mixData[i].srcRaw != 0) {
        // ...
    }
}
```

**Recommended Pattern:**
```cpp
// Define in edgetx.h
extern mutex_handle_t g_model_mutex;

// Use in code
void someFunction() {
    MutexLock lock(g_model_mutex);
    if (g_model.mixData[i].srcRaw != 0) {
        // ...
    }
    // Automatically unlocked on scope exit
}
```

**Best Practice:** Document locking policy:
```cpp
// edgetx.h
/**
 * Global model data
 * 
 * SYNCHRONIZATION: Must hold g_model_mutex for:
 * - All reads in tasks other than main mixer task
 * - All writes
 * 
 * Exception: Mixer task can read without lock (single writer)
 */
extern ModelData g_model;
extern mutex_handle_t g_model_mutex;
```

---

## 6. PRIORITY MIGRATION PLAN

### Phase 1: Critical String Functions (Week 1-2)

**High-traffic files first:**
1. `bluetooth.cpp` - Wireless communication (user-facing)
2. `model_audio.cpp` - File path construction
3. `storage/sdcard_yaml.cpp` - File handling
4. `companion/src/mdichild.cpp` - User data
5. `companion/src/firmwares/radiodata.cpp` - Model data

**Process:**
1. Create `safe_strings.h` helper
2. Migrate one file at a time
3. Test each migration
4. Code review
5. Commit

### Phase 2: File Handle Safety (Week 3)

**All file operations:**
1. Audit all `fopen`, `QFile::open` calls
2. Add error checking
3. Use RAII wrappers where possible
4. Test error paths

### Phase 3: Input Validation (Week 4-5)

**Parsers and external data:**
1. YAML parser validation
2. Telemetry data validation
3. Array bounds checking
4. Zip extraction validation

---

## 7. TESTING STRATEGY

### Unit Tests for Safe Functions

```cpp
// test_safe_strings.cpp
#include "safe_strings.h"
#include <gtest/gtest.h>

TEST(SafeStrings, CopyNormalString) {
    char buf[10];
    EXPECT_TRUE(safe_string_copy(buf, "hello", sizeof(buf)));
    EXPECT_STREQ(buf, "hello");
}

TEST(SafeStrings, CopyTruncatesLongString) {
    char buf[5];
    EXPECT_FALSE(safe_string_copy(buf, "hello world", sizeof(buf)));
    EXPECT_EQ(strlen(buf), 4); // Truncated to fit
    EXPECT_EQ(buf[4], '\0'); // Null terminated
}

TEST(SafeStrings, CopyHandlesNullPointers) {
    char buf[10];
    EXPECT_FALSE(safe_string_copy(nullptr, "test", 10));
    EXPECT_FALSE(safe_string_copy(buf, nullptr, sizeof(buf)));
}
```

### Fuzz Testing for Parsers

```cpp
// fuzz_yaml_parser.cpp
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Create temporary file with fuzz data
    QTemporaryFile file;
    if (file.open()) {
        file.write(reinterpret_cast<const char*>(data), size);
        file.close();
        
        // Try to parse - should not crash
        try {
            YamlParser parser;
            parser.load(file.fileName());
        } catch (...) {
            // Expected for invalid data
        }
    }
    return 0;
}
```

---

## 8. CONTINUOUS INTEGRATION

### Add to GitHub Actions

Create `.github/workflows/security-scan.yml`:

```yaml
name: Security Scan

on: [push, pull_request]

jobs:
  static-analysis:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install tools
        run: |
          sudo apt-get update
          sudo apt-get install -y clang-tools cppcheck
      
      - name: Run Cppcheck
        run: |
          cppcheck --enable=warning,style,performance,portability \
                   --inline-suppr \
                   --error-exitcode=1 \
                   radio/src/ companion/src/ \
                   2> cppcheck-report.txt || true
          cat cppcheck-report.txt
      
      - name: Run Clang-Tidy
        run: |
          cmake -S . -B build \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
          clang-tidy -p build \
            --checks='cert-*,bugprone-*' \
            radio/src/*.cpp
      
      - name: Upload results
        uses: actions/upload-artifact@v3
        with:
          name: security-scan-results
          path: cppcheck-report.txt
```

---

## Summary

**Total Critical Issues:** 8  
**Estimated Fix Time:** 2-3 weeks for one developer  
**Risk Level:** HIGH → MEDIUM after fixes  

**Next Actions:**
1. ✅ Review this document with team
2. ⬜ Create GitHub issues for each critical item
3. ⬜ Implement safe_strings.h
4. ⬜ Begin Phase 1 migration
5. ⬜ Add security scanning to CI

---

*This document provides specific, actionable fixes. For complete review, see [CODE_REVIEW_REPORT.md](./CODE_REVIEW_REPORT.md)*
