cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(yaml-cpp REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json QUIET)

# Include directories
include_directories(include)
include_directories(third_party)

# Add subdirectories
add_subdirectory(src)
# Tests are optional - only build if GTest is available
find_package(GTest QUIET)
if(GTest_FOUND)
    add_subdirectory(tests)
endif()


# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Platform-specific math library
if(UNIX AND NOT APPLE)
    set(MATH_LIB m)
endif()

# Note: Library linking for targets is handled in src/CMakeLists.txt