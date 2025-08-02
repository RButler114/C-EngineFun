# Release Notes Template

## 🎮 C++ Game Engine v{VERSION}

### 🚀 New Features
- Feature 1: Description of new feature
- Feature 2: Description of another feature

### 🐛 Bug Fixes
- Fixed issue with [component] that caused [problem]
- Resolved [specific bug] in [system]

### 📚 Documentation
- Updated API documentation with new examples
- Improved README with better setup instructions
- Added documentation for [new feature]

### 🔧 Technical Changes
- Performance improvements in [system]
- Refactored [component] for better maintainability
- Updated dependencies: SDL2 to version X.X.X

### 💥 Breaking Changes
- [If any] Changed API for [component] - see migration guide below
- [If any] Removed deprecated [feature]

### 📋 Migration Guide
If you're upgrading from a previous version:

```cpp
// Old way (deprecated)
OldAPI::DoSomething();

// New way
NewAPI::DoSomething();
```

### 🧪 Testing
- Added tests for [new features]
- Improved test coverage to X%
- All tests pass on macOS, Ubuntu, and Windows

### 📊 Statistics
- Lines of code: X,XXX
- Documentation coverage: XX%
- Test coverage: XX%
- Supported platforms: macOS, Ubuntu, Windows

### 🔗 Links
- **Documentation**: https://rbutler114.github.io/C-EngineFun/
- **Examples**: See `examples/` directory
- **Getting Started**: See README.md

### 🙏 Contributors
Thanks to all contributors who made this release possible:
- @contributor1
- @contributor2

### 📦 Installation

#### Quick Start
```bash
# Download and extract release
tar -xzf game-engine-v{VERSION}.tar.gz
cd game-engine-v{VERSION}

# Run the demo
chmod +x run_game.sh
./run_game.sh
```

#### Build from Source
```bash
git clone https://github.com/RButler114/C-EngineFun.git
cd C-EngineFun
git checkout v{VERSION}

# Install dependencies
brew install sdl2 sdl2_image pkg-config cmake doxygen  # macOS
# sudo apt-get install libsdl2-dev libsdl2-image-dev pkg-config cmake doxygen  # Ubuntu

# Build
mkdir -p build && cd build
cmake .. && make

# Run
./bin/GameEngine
```

### 🐛 Known Issues
- [If any] Issue description and workaround
- [If any] Platform-specific limitation

### 🔮 What's Next
- Planned feature 1
- Planned feature 2
- Performance improvements

---

**Full Changelog**: https://github.com/RButler114/C-EngineFun/compare/v{PREVIOUS_VERSION}...v{VERSION}
