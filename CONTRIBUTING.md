# Contributing to EdgeTX

Thank you for your interest in contributing to EdgeTX! This document provides guidelines and instructions for contributing to the project.

## Table of Contents
- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Pull Request Process](#pull-request-process)
- [Testing Guidelines](#testing-guidelines)
- [Documentation](#documentation)
- [Community](#community)

## Code of Conduct

EdgeTX is a community-driven project. We expect all contributors to:
- Be respectful and inclusive
- Welcome newcomers and help them get started
- Focus on constructive feedback
- Put the community's interests first

See the [Community Guidelines](https://github.com/EdgeTX/edgetx.github.io/wiki/Community-Guidlines) for more details.

## Getting Started

### Prerequisites

**For Firmware Development:**
- CMake 3.13 or later
- ARM GCC toolchain (arm-none-eabi-gcc)
- Python 3.x (for build tools)
- Git

**For Companion Development:**
- Qt 5.15 or later
- C++11 compatible compiler
- CMake 3.13 or later

**Recommended:**
- Docker (for consistent build environment)
- VSCode or CLion (with CMake support)

### Setting Up Development Environment

1. **Clone the repository:**
   ```bash
   git clone --recursive https://github.com/EdgeTX/edgetx.git
   cd edgetx
   ```

2. **Initialize submodules:**
   ```bash
   git submodule update --init --recursive
   ```

3. **Install dependencies:**
   
   See [BUILD.md](BUILD.md) for detailed platform-specific instructions.

4. **Verify setup:**
   ```bash
   cmake --version
   arm-none-eabi-gcc --version  # For firmware
   qmake --version              # For companion
   ```

### Using Docker (Recommended)

For a consistent build environment:

```bash
# Pull the build environment
docker pull ghcr.io/edgetx/build-edgetx:main

# Build firmware
docker run --rm -v $(pwd):/src ghcr.io/edgetx/build-edgetx:main
```

See [Docker Build Environment](https://github.com/EdgeTX/build-edgetx) for details.

## Development Workflow

### 1. Find or Create an Issue

- Check [existing issues](https://github.com/EdgeTX/edgetx/issues)
- For new features, create an issue first to discuss the approach
- For bugs, provide reproduction steps and expected behavior

### 2. Fork and Branch

```bash
# Fork the repository on GitHub, then:
git clone https://github.com/YOUR_USERNAME/edgetx.git
cd edgetx
git remote add upstream https://github.com/EdgeTX/edgetx.git

# Create a feature branch
git checkout -b feature/my-new-feature
# or for bugs:
git checkout -b fix/issue-description
```

### 3. Make Changes

- Keep commits small and focused
- Write clear commit messages (see [Commit Messages](#commit-messages))
- Test your changes thoroughly
- Update documentation as needed

### 4. Keep Your Branch Updated

```bash
git fetch upstream
git rebase upstream/main
```

### 5. Push and Create Pull Request

```bash
git push origin feature/my-new-feature
```

Then create a Pull Request on GitHub.

## Coding Standards

### C/C++ Code Style

EdgeTX uses a specific code style enforced by `.clang-format`.

**Basic Rules:**
- **Indentation**: 2 spaces (no tabs)
- **Braces**: Opening brace on same line
- **Naming**:
  - Variables: `camelCase`
  - Functions: `camelCase()`
  - Classes: `PascalCase`
  - Constants: `UPPER_CASE`
  - Macros: `UPPER_CASE`
- **Line Length**: Maximum 120 characters (prefer 80-100)

**Format your code:**
```bash
# Format a single file
clang-format -i path/to/file.cpp

# Format all changed files
git diff --name-only | grep -E '\.(cpp|h)$' | xargs clang-format -i
```

### Code Organization

**Header Files:**
```cpp
#ifndef FILE_NAME_H
#define FILE_NAME_H

// Includes
#include <system_headers>
#include "local_headers.h"

// Constants and macros
#define MAX_VALUE 100

// Type declarations
struct MyStruct {
  int field;
};

// Function declarations
void myFunction();

#endif // FILE_NAME_H
```

**Source Files:**
```cpp
#include "header.h"

// Static/local functions
static void helperFunction() {
  // Implementation
}

// Public functions
void myFunction() {
  // Implementation
}
```

### Comments

- Use `//` for single-line comments
- Use `/* */` for multi-line comments
- Document complex algorithms and non-obvious behavior
- Avoid obvious comments
- Keep comments up-to-date with code changes

**Good:**
```cpp
// Calculate exponential curve with midpoint adjustment
int applyExpo(int input, int expo, int midpoint) {
  // ...
}
```

**Bad:**
```cpp
// Set i to 0
int i = 0;
```

### Error Handling

- Check return values from functions
- Validate input parameters
- Use assert for debugging checks (not in production logic)
- Handle resource cleanup properly (RAII preferred)

### Memory Management

- Minimize dynamic allocation in firmware (limited heap)
- Use static or stack allocation where possible
- Clean up resources in destructors
- Avoid memory leaks

## Pull Request Process

### Before Submitting

- [ ] Code compiles without warnings
- [ ] Tests pass (if applicable)
- [ ] Code formatted with clang-format
- [ ] Documentation updated
- [ ] Commit messages follow guidelines
- [ ] Branch rebased on latest main

### PR Title and Description

**Title Format:**
```
<type>(<scope>): <description>

Examples:
feat(mixer): add curve smoothing option
fix(audio): resolve click on startup
docs(contributing): update coding standards
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style/formatting
- `refactor`: Code restructuring
- `perf`: Performance improvement
- `test`: Adding tests
- `chore`: Build/tooling changes

**Description Template:**
```markdown
## Description
Brief description of changes

## Motivation
Why is this change needed?

## Changes Made
- Item 1
- Item 2

## Testing
How was this tested?

## Screenshots (if applicable)
[Add screenshots for UI changes]

## Related Issues
Fixes #123
Related to #456
```

### Review Process

1. **Automated Checks**: CI/CD runs automatically
2. **Code Review**: Maintainers review the code
3. **Feedback**: Address review comments
4. **Approval**: At least one maintainer approval required
5. **Merge**: Maintainer merges when ready

### Addressing Review Comments

```bash
# Make changes based on feedback
git add .
git commit -m "address review comments"
git push origin feature/my-new-feature
```

## Commit Messages

We follow [Conventional Commits](https://conventionalcommits.org/) specification.

**Format:**
```
<type>(<scope>): <subject>

<body>

<footer>
```

**Example:**
```
fix(mixer): prevent crash on empty curve

The mixer would crash when a curve with no points was referenced.
Added validation to check for minimum 2 points before processing.

Fixes #1234
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation
- `style`: Formatting
- `refactor`: Code restructuring
- `perf`: Performance
- `test`: Tests
- `chore`: Build/tooling

**Scopes:** (component affected)
- `mixer`, `audio`, `telemetry`, `lua`, `gui`, `storage`, etc.
- `companion`, `simulator`
- `build`, `ci`

## Testing Guidelines

### Firmware Testing

**Simulator Testing:**
```bash
# Build simulator
cmake --preset default
cmake --build build-default --target simulator

# Run simulator
./build-default/native/simulator
```

**Unit Tests:**
```bash
# Build and run tests
cmake --build build-default --target gtests-radio
./build-default/native/gtests-radio
```

**Hardware Testing:**
1. Flash firmware to radio
2. Test basic operations (inputs, mixer, audio)
3. Test specific feature changes
4. Test with multiple models
5. Verify no regressions

### Companion Testing

```bash
# Build companion
cmake --build build-default --target companion

# Run companion
./build-default/native/companion
```

**Test Areas:**
- Model editor (create, edit, save)
- Firmware flasher
- Backup/restore
- Simulator integration
- Multiple radio types

### Writing Tests

For firmware, use GoogleTest framework:

```cpp
#include "gtest/gtest.h"

TEST(MixerTest, BasicCurve) {
  // Arrange
  int input = 512;
  
  // Act
  int result = applyCurve(input, CURVE_LINEAR);
  
  // Assert
  EXPECT_EQ(result, 512);
}
```

## Documentation

### Code Documentation

- Document public APIs and complex functions
- Use Doxygen-style comments for API documentation
- Keep documentation close to code

**Example:**
```cpp
/**
 * @brief Apply exponential curve to input value
 * 
 * @param value Input value (-1024 to 1024)
 * @param expo Exponential factor (0 to 100)
 * @return Transformed value
 */
int applyExpo(int value, int expo);
```

### User Documentation

- Update user manual for feature changes
- Add screenshots for UI changes
- Update Lua documentation for API changes
- Keep README.md current

### Where to Document

- **Code Changes**: Inline comments and function documentation
- **Architecture**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **Build Process**: [BUILD.md](BUILD.md)
- **API Reference**: [API_REFERENCE.md](API_REFERENCE.md)
- **User Features**: EdgeTX Manual (separate repository)
- **Lua Scripts**: https://luadoc.edgetx.org/

## Community

### Communication Channels

- **Discord**: https://discord.gg/wF9wUKnZ6H (main chat)
- **Facebook**: https://www.facebook.com/groups/edgetx
- **GitHub Discussions**: For long-form topics
- **GitHub Issues**: For bug reports and feature requests

### Getting Help

- Check existing documentation first
- Search closed issues for similar problems
- Ask in Discord #development channel
- Create a discussion for broad questions
- Create an issue for specific bugs

### Becoming a Contributor

1. **Start Small**: Fix typos, improve docs, fix small bugs
2. **Learn the Codebase**: Read code, run simulator, ask questions
3. **Take on Issues**: Look for "good first issue" label
4. **Review PRs**: Help review others' pull requests
5. **Engage**: Participate in discussions, help others

### Recognition

Contributors are recognized through:
- GitHub contributors list
- Release notes
- Community acknowledgment
- Maintainer nomination (for long-term contributors)

## Development Tips

### Debugging

**Simulator:**
- Use debugger (gdb/lldb) with simulator build
- Add printf/TRACE statements
- Use Qt Creator for GUI debugging

**Hardware:**
- Use JTAG/SWD debugger (ST-Link, J-Link)
- Serial debug output (where available)
- Telemetry for field debugging
- LED indicators

### Performance Considerations

- Mixer runs at 50Hz or higher (critical timing)
- Minimize blocking operations in main loop
- Use DMA for I/O operations
- Profile before optimizing

### Common Pitfalls

- **Stack Overflow**: Limited stack in firmware (use heap or static)
- **Timing**: Don't block the mixer scheduler
- **Endianness**: Be careful with multi-byte data
- **Compatibility**: Test with multiple radio types
- **Storage**: EEPROM/Flash have limited write cycles

## Build System Specifics

### CMake Options

Common options for firmware builds:

```bash
# Choose radio target
cmake -DPCB=X10 ...

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ...

# With Lua support
cmake -DLUA=YES ...
```

See [BUILD.md](BUILD.md) for complete list.

### Target Selection

Available in `fw.json`:
- `PCB` option: Radio board (X10, X12S, T16, etc.)
- `PCBREV` option: Board revision
- `INTERNAL_MODULE_MULTI`: Multi-protocol support

## Version Control

### Branch Strategy

- `main`: Stable development branch
- `v2.x`: Maintenance branches for releases
- `feature/*`: Feature development
- `fix/*`: Bug fixes

### Rebasing vs Merging

- **Prefer rebasing** to keep history clean
- **Don't rebase** public/shared branches
- **Squash** small fixup commits before merging

### Commit Best Practices

- One logical change per commit
- Commit working code (don't break builds)
- Write meaningful commit messages
- Reference issues in commits

## Release Process

(For maintainers)

1. Version bump in CMakeLists.txt
2. Update CHANGELOG
3. Create release branch
4. Final testing
5. Tag release (vX.Y.Z)
6. Build release binaries
7. Create GitHub release
8. Announce release

## License

By contributing to EdgeTX, you agree that your contributions will be licensed under the [GPL v2 License](LICENSE).

## Questions?

- Read the docs: [ARCHITECTURE.md](ARCHITECTURE.md), [BUILD.md](BUILD.md)
- Ask on Discord: https://discord.gg/wF9wUKnZ6H
- GitHub Discussions: https://github.com/EdgeTX/edgetx/discussions

Thank you for contributing to EdgeTX! 🎉
