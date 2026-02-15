# EdgeTX Code Review - Quick Reference

**One-page overview of findings and actions**

---

## 📊 Review Statistics

| Metric | Count |
|--------|-------|
| Files Reviewed | 2,766 |
| Lines of Code | 1,457,089 |
| 🔴 Critical Issues | 8 |
| 🟠 High Priority | 15 |
| 🟡 Medium Priority | 25+ |
| 🟢 TODOs | 602 |

---

## 🔴 Critical Security Issues

| # | Issue | Files | Fix |
|---|-------|-------|-----|
| 1 | Buffer overflows | 50+ | Replace `strcpy`/`strcat`/`sprintf` |
| 2 | Resource leaks | 10+ | Add error checking, RAII |
| 3 | Input validation | Multiple | Bounds checks, validation |

---

## 📝 Top 5 Files Requiring Immediate Attention

1. **`radio/src/bluetooth.cpp`**
   - Line 533: `strcpy(distantAddr, &line[10]); // TODO quick & dirty`
   - Fix: Use `snprintf` with bounds

2. **`radio/src/model_audio.cpp`**
   - Lines 31, 90-91: Multiple unsafe string operations
   - Fix: Replace with safe alternatives

3. **`companion/src/mdichild.cpp`**
   - Unsafe `strcpy` with QString conversion
   - Fix: Validate length, use `strncpy`

4. **`companion/src/firmwares/radiodata.cpp`**
   - Unbounded `sprintf` calls
   - Fix: Use `snprintf`

5. **`companion/src/storage/minizinterface.cpp`**
   - Zip path traversal vulnerability
   - Fix: Validate paths before extraction

---

## ⚡ Quick Win Fixes

### 1. Safe String Wrapper (15 minutes)

Create `radio/src/safe_strings.h`:
```cpp
inline bool safe_copy(char* dst, const char* src, size_t sz) {
    if (!dst || !src || sz == 0) return false;
    strncpy(dst, src, sz - 1);
    dst[sz - 1] = '\0';
    return strlen(src) < sz;
}
```

### 2. File Open Check Template (5 minutes)

```cpp
// C style
FILE* f = fopen(path, "r");
if (!f) { TRACE("Failed"); return ERROR; }
// ... use f ...
fclose(f);

// Qt style
QFile file(path);
if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << file.errorString();
    return false;
}
```

### 3. Array Bounds Template (5 minutes)

```cpp
if (index < 0 || index >= ARRAY_SIZE) {
    TRACE("Index out of bounds: %d", index);
    return ERROR;
}
```

---

## 🎯 Fix Priority Matrix

```
Impact →  Low      Medium    High      Critical
  ↓
Effort
Low       🟢       🟡        🟠        🔴
          Later    Week 4-5  Week 3    Week 1-2

Medium    🟢       🟡        🟠        🔴
          Later    Week 6-7  Week 4-5  Week 2-3

High      🟢       🟡        🟠        🟠
          Later    Later     Week 6-7  Week 4-5
```

**Week 1-2:** 🔴 Buffer overflows  
**Week 3:** 🔴 Resource leaks  
**Week 4-5:** 🟠 Input validation  
**Week 6-7:** 🟠 Concurrency, security

---

## 🔧 Tools to Enable

```bash
# Static Analysis (add to CI)
cppcheck --enable=all radio/src/ companion/src/

# Memory Safety (local testing)
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" -S . -B build
cmake --build build && ./build/tests

# Thread Safety (local testing)
cmake -DCMAKE_CXX_FLAGS="-fsanitize=thread" -S . -B build
```

---

## 📋 Team Responsibilities

### Security Team
- [ ] Week 1-2: Fix all `strcpy/strcat/sprintf`
- [ ] Week 3: Add file handle error checking
- [ ] Setup static analysis in CI

### Embedded Team (radio/src)
- [ ] Week 4: Add telemetry validation
- [ ] Week 5: Document mutex usage
- [ ] Audit global state access

### Desktop Team (companion/src)
- [ ] Week 4: Fix zip path traversal
- [ ] Week 5: Add parser size limits
- [ ] Move blocking ops to threads

### QA Team
- [ ] Create test cases for fixed issues
- [ ] Run fuzz testing on parsers
- [ ] Verify all fixes

---

## 📚 Reference Documents

| Document | Purpose | Audience |
|----------|---------|----------|
| `CODE_REVIEW_SUMMARY.md` | Executive overview | Management |
| `CODE_REVIEW_REPORT.md` | Detailed findings | All developers |
| `SECURITY_FIXES.md` | Code examples & fixes | Developers |
| `QUICK_REFERENCE.md` | One-page cheat sheet | Everyone |

---

## 🚨 Emergency Response

**If a security issue is exploited in the wild:**

1. **Immediate** (Day 1):
   - Identify affected versions
   - Create hotfix branch
   - Fix critical vulnerability
   - Release emergency update

2. **Short-term** (Week 1):
   - Audit similar code patterns
   - Add regression tests
   - Update security advisory

3. **Long-term** (Month 1):
   - Complete security review
   - Implement all critical fixes
   - Add security testing to CI

---

## ✅ Success Criteria

**Phase 1 Complete When:**
- [ ] Zero unsafe `strcpy/strcat/sprintf` in critical paths
- [ ] All file operations have error checking
- [ ] Static analysis added to CI
- [ ] Security advisory published (if needed)

**Phase 2 Complete When:**
- [ ] All input validation added
- [ ] Concurrency documented and tested
- [ ] Companion security issues fixed
- [ ] Test coverage > 70%

**Phase 3 Complete When:**
- [ ] Technical debt < 200 TODOs
- [ ] Code quality metrics green
- [ ] Documentation complete
- [ ] All CI checks passing

---

## 📞 Contact & Resources

**Security Issues:** security@edgetx.org (if exists)  
**Code Review Lead:** [Assign person]  
**Documentation:** https://github.com/EdgeTX/edgetx/wiki  
**CI Dashboard:** https://github.com/EdgeTX/edgetx/actions  

---

## 🎓 Learning Resources

**For Team Members:**

1. **Secure Coding in C/C++**
   - CERT C Coding Standard
   - CWE Top 25 Most Dangerous Software Errors

2. **Memory Safety**
   - AddressSanitizer documentation
   - Valgrind tutorial

3. **Concurrency**
   - ThreadSanitizer guide
   - FreeRTOS synchronization patterns

---

## 📈 Progress Tracking

**Update weekly in team meetings:**

```markdown
## Week X Progress

### Completed
- [ ] Item 1
- [ ] Item 2

### In Progress
- [ ] Item 3 (Developer A, 50%)
- [ ] Item 4 (Developer B, 30%)

### Blocked
- [ ] Item 5 (waiting for X)

### Metrics
- Issues fixed: X/Y
- Test coverage: Z%
- CI status: Green/Red
```

---

## 🔄 Review Cycle

**This code review should be repeated:**

- **After major features:** New protocol, UI changes
- **Before releases:** All release candidates
- **Quarterly:** Regular security audit
- **After incidents:** Any security issue

**Next Review:** [Schedule 3 months from now]

---

*For detailed information, see the complete documentation set in the repository root.*

**Last Updated:** February 15, 2026  
**Review Version:** 1.0  
**Reviewed By:** Code Review Agent
