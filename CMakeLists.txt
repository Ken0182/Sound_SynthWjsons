cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags - set before adding subdirectories
# Use more portable flags for cross-platform compatibility
if(MSYS OR MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -ffast-math")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Find required packages with better error handling
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(JSONCPP jsoncpp)
endif()

# Fallback to find_package if pkg-config didn't work
if(NOT JSONCPP_FOUND)
    find_package(jsoncpp CONFIG)
    if(jsoncpp_FOUND)
        set(JSONCPP_LIBRARIES jsoncpp_lib)
        set(JSONCPP_INCLUDE_DIRS ${jsoncpp_INCLUDE_DIRS})
    else()
        message(WARNING "jsoncpp not found via pkg-config or find_package. Trying manual detection...")
        find_path(JSONCPP_INCLUDE_DIRS json/json.h)
        find_library(JSONCPP_LIBRARIES NAMES jsoncpp)
        if(NOT JSONCPP_INCLUDE_DIRS OR NOT JSONCPP_LIBRARIES)
            message(FATAL_ERROR "jsoncpp is required but not found. Please install it:\n"
                               "  Linux: sudo apt-get install libjsoncpp-dev\n"
                               "  macOS: brew install jsoncpp\n"
                               "  Windows: Use vcpkg or download from https://github.com/open-source-parsers/jsoncpp")
        endif()
    endif()
endif()

# Find yaml-cpp with better error handling
find_package(yaml-cpp CONFIG)
if(NOT yaml-cpp_FOUND)
    find_package(yaml-cpp)
    if(NOT yaml-cpp_FOUND AND PkgConfig_FOUND)
        pkg_check_modules(YAML_CPP yaml-cpp)
        if(YAML_CPP_FOUND)
            set(YAML_CPP_LIBRARIES ${YAML_CPP_LIBRARIES})
            set(YAML_CPP_INCLUDE_DIRS ${YAML_CPP_INCLUDE_DIRS})
        endif()
    endif()
    if(NOT yaml-cpp_FOUND AND NOT YAML_CPP_FOUND)
        message(FATAL_ERROR "yaml-cpp is required but not found. Please install it:\n"
                           "  Linux: sudo apt-get install libyaml-cpp-dev\n"
                           "  macOS: brew install yaml-cpp\n"
                           "  Windows: Use vcpkg or download from https://github.com/jbeder/yaml-cpp")
    endif()
endif()

# Include directories
include_directories(include)
include_directories(third_party)
include_directories(${JSONCPP_INCLUDE_DIRS})

# Add subdirectories
add_subdirectory(src)

# Tests are optional - only build if GTest is available
find_package(GTest QUIET)
if(GTest_FOUND)
    add_subdirectory(tests)
endif()