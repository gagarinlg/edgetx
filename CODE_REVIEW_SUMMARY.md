# EdgeTX Code Review - Executive Summary

**Date:** February 15, 2026  
**Review Scope:** Complete analysis of `radio/src` and `companion/src` directories  
**Code Size:** ~1.5M lines across 2,766 source files

---

## Quick Stats

| Metric | Value |
|--------|-------|
| Total Lines of Code | 1,457,089 |
| Source Files Reviewed | 2,766 |
| Critical Issues | 8 |
| High Priority Issues | 15 |
| Medium Priority Issues | 25+ |
| Technical Debt Items (TODOs) | 602+ |

---

## Critical Issues Requiring Immediate Attention 🔴

### 1. Buffer Overflow Vulnerabilities

**Impact:** HIGH - Could lead to crashes or security exploits  
**Affected:** 50+ locations across codebase  
**Root Cause:** Use of unsafe C string functions (`strcpy`, `strcat`, `sprintf`)

**Top 5 Critical Files:**
1. `radio/src/bluetooth.cpp` - Line 533: `strcpy(distantAddr, &line[10]); // TODO quick & dirty`
2. `radio/src/model_audio.cpp` - Lines 31, 90, 91: Multiple unsafe operations
3. `companion/src/mdichild.cpp` - `strcpy` without bounds checking
4. `companion/src/firmwares/radiodata.cpp` - Unbounded `sprintf` calls
5. `radio/src/audio.cpp` - Multiple unsafe string operations

**Action Required:**
- Replace all `strcpy` with `strncpy` or `strlcpy`
- Replace all `strcat` with `strncat` or `strlcat`  
- Replace all `sprintf` with `snprintf`
- Create safe wrapper functions for common operations

**Effort:** 2-3 weeks (1 developer)

---

### 2. Resource Leaks (File Handles)

**Impact:** MEDIUM-HIGH - Memory/resource exhaustion  
**Affected:** 10+ file I/O locations

**Key Files:**
- `radio/src/targets/simu/simudisk.cpp:41` - Unchecked `fopen()`
- `companion/src/print/modelprinter.cpp:28-29` - Unchecked file open
- `companion/src/storage/` - Multiple files missing error checks

**Action Required:**
- Add error checking to all file operations
- Use RAII patterns for automatic cleanup
- Implement consistent error handling

**Effort:** 1-2 weeks (1 developer)

---

### 3. Input Validation Gaps

**Impact:** MEDIUM - Potential crashes from malformed data  
**Affected:** File parsers, telemetry handlers, user inputs

**Key Areas:**
- YAML parser - Missing validation (602+ TODO markers)
- Telemetry data - `reinterpret_cast` without validation
- Array access - Missing bounds checks in several locations

**Action Required:**
- Add bounds checking for all array accesses
- Validate telemetry data before casting
- Complete validation TODOs in parsers

**Effort:** 2-3 weeks (1 developer)

---

## High Priority Issues 🟠

### 4. Concurrency Concerns

- 479 accesses to global state (`g_model`, `g_eeGeneral`)
- Inconsistent mutex usage
- Potential race conditions

**Action:** Document synchronization strategy, audit mutex usage

---

### 5. Companion Security Issues

- Zip path traversal vulnerability in `minizinterface.cpp`
- YAML parser without size limits (DoS risk)
- No file size limits before parsing

**Action:** Add path validation, size limits, timeouts

---

### 6. UI Responsiveness

- Blocking operations with `processEvents()` 
- Long-running operations in UI thread

**Action:** Move to background threads using QThread

---

## Positive Findings ✅

1. ✅ Well-structured HAL abstraction
2. ✅ Good test infrastructure with simulation
3. ✅ Proper Qt parent ownership (mostly)
4. ✅ Active community and regular updates
5. ✅ CMake build system with feature flags
6. ✅ Consistent coding patterns

---

## Recommended Action Plan

### Phase 1: Critical Security Fixes (Weeks 1-3)

**Week 1-2: String Safety**
- [ ] Audit all `strcpy`, `strcat`, `sprintf` calls
- [ ] Create safe wrapper functions
- [ ] Migrate critical paths (user input, file I/O)
- [ ] Add unit tests for string operations

**Week 3: Resource Management**
- [ ] Fix all file handle leaks
- [ ] Add error checking to file operations
- [ ] Implement RAII patterns where needed

### Phase 2: High Priority Fixes (Weeks 4-9)

**Week 4-5: Input Validation**
- [ ] Add bounds checking for array access
- [ ] Validate telemetry data
- [ ] Add fuzz testing for parsers

**Week 6-7: Concurrency**
- [ ] Document synchronization strategy
- [ ] Audit global state access
- [ ] Add ThreadSanitizer builds

**Week 8-9: Companion Security**
- [ ] Fix zip path traversal
- [ ] Add parser size limits
- [ ] Move blocking ops to threads

### Phase 3: Code Quality (Ongoing)

**Technical Debt Reduction:**
- [ ] Prioritize 602 TODO items
- [ ] Remove dead code
- [ ] Replace magic numbers with constants
- [ ] Break up complex functions

**Testing Improvements:**
- [ ] Add static analysis to CI
- [ ] Increase test coverage
- [ ] Add fuzz testing

**Documentation:**
- [ ] Add architecture docs
- [ ] Document synchronization
- [ ] Improve API docs

---

## Tools to Enable

### Static Analysis
```bash
# Clang Static Analyzer
scan-build cmake -S . -B build
scan-build make -C build

# Cppcheck
cppcheck --enable=all radio/src/ companion/src/
```

### Dynamic Analysis
```bash
# AddressSanitizer (memory issues)
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" -S . -B build-asan

# ThreadSanitizer (race conditions)
cmake -DCMAKE_CXX_FLAGS="-fsanitize=thread" -S . -B build-tsan
```

### Code Formatting
```bash
# Already has .clang-format - use it!
find radio/src companion/src -name "*.cpp" | xargs clang-format -i
```

---

## Risk Assessment

| State | Risk Level | Description |
|-------|-----------|-------------|
| **Current** | 🔴 HIGH | Buffer overflows, resource leaks present |
| **After Phase 1** | 🟡 MEDIUM | Critical security issues fixed |
| **After Phase 2** | 🟢 LOW | Most issues addressed |
| **After Phase 3** | 🟢 LOW | Technical debt managed |

---

## Estimated Total Effort

- **Critical Fixes (Phase 1):** 3 weeks (1 developer)
- **High Priority (Phase 2):** 6 weeks (1 developer)
- **Code Quality (Phase 3):** Ongoing (shared across team)

**Total for critical and high priority:** ~9 weeks for 1 developer, or ~3 weeks for 3 developers working in parallel

---

## Key Takeaways

1. **EdgeTX is a mature project** with good architecture and community
2. **Critical security issues exist** but are fixable with focused effort
3. **Technical debt is typical** for a project of this size and history
4. **Strong foundation** - good testing and build infrastructure
5. **Clear path forward** - issues are well-understood and actionable

**Bottom Line:** The codebase is solid but needs security hardening and technical debt reduction. With systematic effort, it can achieve high quality and security standards.

---

## Next Steps

1. **Review this report** with the development team
2. **Prioritize issues** based on actual usage patterns
3. **Create GitHub issues** for tracked work items
4. **Set up CI** with static analysis tools
5. **Begin Phase 1** security fixes immediately

---

*For detailed findings and code examples, see the full [CODE_REVIEW_REPORT.md](./CODE_REVIEW_REPORT.md)*
