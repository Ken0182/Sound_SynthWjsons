# Third-Party Dependencies

This directory contains bundled header-only libraries for easier project setup.

## nlohmann/json

- **Version**: 3.11.3
- **License**: MIT
- **Homepage**: https://github.com/nlohmann/json
- **Purpose**: Modern C++ JSON library (header-only)

The single-header version (`nlohmann/json.hpp`) is included for convenience.

### Updating

To update to the latest version:

```bash
curl -o third_party/nlohmann/json.hpp \
  https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
```

Or a specific version:

```bash
curl -o third_party/nlohmann/json.hpp \
  https://raw.githubusercontent.com/nlohmann/json/v3.11.3/single_include/nlohmann/json.hpp
```

## System Packages (Recommended)

While bundled headers are provided, we recommend installing system packages when available:

### Ubuntu/Debian
```bash
sudo apt-get install nlohmann-json3-dev
```

### macOS
```bash
brew install nlohmann-json
```

### Windows (MSYS2)
```bash
pacman -S mingw-w64-x86_64-nlohmann-json
```

### Windows (vcpkg)
```bash
vcpkg install nlohmann-json:x64-windows
```

The CMake build system will automatically use system packages if available, falling back to bundled headers otherwise.
