# EdgeTX Documentation

Welcome to the EdgeTX documentation! This directory contains comprehensive documentation for developers and contributors.

## Documentation Structure

### Getting Started
- [README.md](../README.md) - Project overview and quick links
- [BUILD.md](../BUILD.md) - Complete build instructions for all platforms
- [CONTRIBUTING.md](../CONTRIBUTING.md) - How to contribute to EdgeTX

### Architecture & Design
- [ARCHITECTURE.md](../ARCHITECTURE.md) - System architecture and design overview
- [DEVELOPER_GUIDE.md](../DEVELOPER_GUIDE.md) - Developer workflows, debugging, and best practices
- [API_REFERENCE.md](../API_REFERENCE.md) - API documentation for major subsystems

### Hardware
- [HARDWARE_SUPPORT.md](../HARDWARE_SUPPORT.md) - Guide for adding new radio hardware

## Quick Links

### For New Contributors
1. Start with [ARCHITECTURE.md](../ARCHITECTURE.md) to understand the system
2. Read [BUILD.md](../BUILD.md) to set up your development environment
3. Follow [CONTRIBUTING.md](../CONTRIBUTING.md) for workflow and standards
4. Use [DEVELOPER_GUIDE.md](../DEVELOPER_GUIDE.md) for debugging and testing

### For Hardware Developers
1. Read [HARDWARE_SUPPORT.md](../HARDWARE_SUPPORT.md) for porting guide
2. Study existing board definitions in `radio/src/boards/`
3. Review target configurations in `radio/src/targets/`

### For Firmware Developers
1. Understand the architecture in [ARCHITECTURE.md](../ARCHITECTURE.md)
2. Use [API_REFERENCE.md](../API_REFERENCE.md) for API details
3. Follow debugging techniques in [DEVELOPER_GUIDE.md](../DEVELOPER_GUIDE.md)

### For Companion Developers
1. See Companion architecture in [ARCHITECTURE.md](../ARCHITECTURE.md)
2. Review source code in `companion/src/`
3. Qt documentation: https://doc.qt.io/

## External Documentation

- **User Manual**: https://manual.edgetx.org/
- **Lua Documentation**: https://luadoc.edgetx.org/
- **Development Wiki**: https://github.com/EdgeTX/edgetx/wiki
- **Community Discord**: https://discord.gg/wF9wUKnZ6H

## Documentation Topics

### Build System
- CMake configuration and presets
- Cross-compilation for ARM targets
- Building firmware, companion, and simulator
- Docker-based builds

### Firmware
- Mixer system and channel processing
- Audio engine and voice prompts
- Telemetry protocols and data handling
- Lua scripting integration
- GUI system for different screen types
- Storage and file system
- RF protocols and modules

### Companion
- Model editor and data structures
- Firmware interface and versioning
- Simulator integration
- Backup and restore functionality

### Testing
- Unit testing with GoogleTest
- Simulator-based testing
- Hardware testing procedures
- Continuous integration

### Code Quality
- Coding standards and style
- Code review process
- Performance optimization
- Memory management

## Contributing to Documentation

Documentation improvements are always welcome! If you find errors or have suggestions:

1. Create an issue describing the problem
2. Or submit a pull request with fixes
3. Follow markdown formatting guidelines
4. Keep documentation in sync with code

### Documentation Guidelines

- **Clear and Concise**: Write for developers of varying skill levels
- **Code Examples**: Include practical examples where relevant
- **Keep Updated**: Update docs when code changes
- **Link Related**: Cross-reference related documentation
- **Diagrams**: Use ASCII diagrams or mermaid syntax where helpful

## Need Help?

- Check existing documentation first
- Search [GitHub Issues](https://github.com/EdgeTX/edgetx/issues)
- Ask in [Discord](https://discord.gg/wF9wUKnZ6H) #development channel
- Create a [GitHub Discussion](https://github.com/EdgeTX/edgetx/discussions)

Thank you for contributing to EdgeTX! 🚀
