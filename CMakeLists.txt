cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# Use simple JSON parser (header-only)
message(STATUS "Using simple JSON parser")

# Optional packages
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    add_definitions(-DHAVE_OPENCV)
else()
    message(STATUS "OpenCV not found - some features will be disabled")
endif()

find_package(yaml-cpp QUIET)
if(yaml-cpp_FOUND)
    message(STATUS "yaml-cpp found: ${yaml-cpp_VERSION}")
    add_definitions(-DHAVE_YAML_CPP)
else()
    message(STATUS "yaml-cpp not found - YAML features will be disabled")
endif()

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")

# Platform-specific settings
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()

# Create presets for different build configurations
set(CMAKE_PRESETS_FILE "${CMAKE_SOURCE_DIR}/CMakePresets.json" CACHE STRING "Path to CMake presets file")

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)