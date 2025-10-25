## MSYS2 / MinGW Build Notes

- Ensure MSYS2 packages are installed: `mingw-w64-x86_64-gcc`, `mingw-w64-x86_64-cmake`, `mingw-w64-x86_64-pkgconf`, `mingw-w64-x86_64-yaml-cpp`, `mingw-w64-x86_64-jsoncpp`, `mingw-w64-x86_64-pybind11`.
- Configure with: `cmake --preset msys2-mingw` and build with: `cmake --build --preset msys2-mingw`.
- Python bindings are optional; they build if pybind11 is installed.
