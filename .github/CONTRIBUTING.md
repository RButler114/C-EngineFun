# Contributing to C++ Game Engine

Thank you for your interest in contributing to our C++ Game Engine! This document provides guidelines and information for contributors.

## 🚀 Quick Start

1. **Fork the repository** and clone your fork
2. **Install dependencies** following the README.md instructions
3. **Create a feature branch** from `main`
4. **Make your changes** with proper documentation
5. **Test your changes** thoroughly
6. **Submit a pull request**

## 📋 Development Setup

### Prerequisites
- CMake 3.16+
- C++17 compatible compiler (Clang/GCC/MSVC)
- SDL2 and SDL2_image libraries
- Doxygen (for documentation)

### Build and Test
```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/C-EngineFun.git
cd C-EngineFun

# Build
mkdir -p build && cd build
cmake .. && make

# Run tests
cd ../test && ./run_tests.sh

# Generate documentation
./generate_docs.sh
```

## 🎯 Contribution Guidelines

### Code Style
- Follow existing code style and conventions
- Use meaningful variable and function names
- Add Doxygen comments for all public APIs
- Keep functions focused and reasonably sized

### Documentation Requirements
All contributions must include:
- **Doxygen comments** for new classes, functions, and significant changes
- **Code examples** for new features
- **Updated README.md** if adding new functionality
- **Test documentation** for new test cases

### Example Documentation:
```cpp
/**
 * @brief Brief description of the function
 * 
 * Detailed description of what the function does,
 * when to use it, and any important considerations.
 * 
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Description of return value
 * 
 * @example
 * ```cpp
 * MyClass instance;
 * bool result = instance.MyFunction(value1, value2);
 * ```
 */
bool MyFunction(int param1, const std::string& param2);
```

## 🧪 Testing

### Required Tests
- **Unit tests** for new functionality
- **Integration tests** for system interactions
- **Manual testing** on target platforms

### Test Structure
```bash
test/
├── run_tests.sh              # Main test runner
├── test_basic_rendering.cpp  # Core rendering tests
├── test_input_system.cpp     # Input handling tests
├── test_menu_rendering.cpp   # UI/menu tests
└── test_state_management.cpp # Game state tests
```

## 🏗️ Architecture Guidelines

### ECS System
- **Entities** are lightweight IDs
- **Components** are pure data (no behavior)
- **Systems** contain all logic
- Follow existing patterns in `src/ECS/`

### Engine Core
- Keep platform-specific code isolated
- Use RAII for resource management
- Prefer composition over inheritance
- Follow existing patterns in `src/Engine/`

## 📚 Documentation Standards

### File Documentation
Every header file should have:
```cpp
/**
 * @file FileName.h
 * @brief Brief description of the file's purpose
 * @author Your Name
 * @date 2025
 */
```

### Class Documentation
```cpp
/**
 * @class ClassName
 * @brief Brief description of the class
 * 
 * Detailed description of the class purpose,
 * usage patterns, and important notes.
 * 
 * @example
 * ```cpp
 * ClassName instance;
 * instance.DoSomething();
 * ```
 */
```

## 🔄 Pull Request Process

1. **Create descriptive PR title** following format: `[TYPE] Brief description`
   - Types: `[FEATURE]`, `[BUG]`, `[DOCS]`, `[REFACTOR]`, `[TEST]`

2. **Fill out PR template** completely

3. **Ensure CI passes**:
   - Build succeeds on all platforms
   - All tests pass
   - Documentation generates successfully
   - Code quality checks pass

4. **Request review** from maintainers

5. **Address feedback** promptly and professionally

## 🐛 Bug Reports

Use the bug report template and include:
- Clear reproduction steps
- Expected vs actual behavior
- Environment details (OS, compiler, etc.)
- Console output and error messages

## ✨ Feature Requests

Use the feature request template and include:
- Clear description of the feature
- Use cases and motivation
- Example usage code
- Implementation considerations

## 📞 Getting Help

- **Documentation**: Check the generated docs at `docs/html/index.html`
- **Issues**: Search existing issues before creating new ones
- **Discussions**: Use GitHub Discussions for questions
- **Code Review**: Don't hesitate to ask for clarification during review

## 🎉 Recognition

Contributors will be:
- Listed in the project contributors
- Mentioned in release notes for significant contributions
- Credited in documentation for major features

Thank you for contributing to making this game engine better! 🎮✨
