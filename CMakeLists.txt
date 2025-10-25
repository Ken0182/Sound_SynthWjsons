cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(Threads REQUIRED)
find_package(yaml-cpp QUIET CONFIG)
if(yaml-cpp_FOUND)
    set(AIAUDIO_HAVE_YAML ON)
    add_compile_definitions(AIAUDIO_ENABLE_YAML)
else()
    set(AIAUDIO_HAVE_YAML OFF)
endif()

# JsonCpp detection (pkg-config, then CMake config)
set(JSONCPP_LIBS "")
set(JSONCPP_INCLUDE_DIRS "")
set(AIAUDIO_HAVE_JSONCPP OFF)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(JSONCPP QUIET jsoncpp)
    if(JSONCPP_FOUND)
        set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIRS})
        set(JSONCPP_LIBS ${JSONCPP_LIBRARIES})
        set(AIAUDIO_HAVE_JSONCPP ON)
    endif()
endif()

if(NOT AIAUDIO_HAVE_JSONCPP)
    find_package(JsonCpp QUIET CONFIG)
    if(JsonCpp_FOUND)
        set(JSONCPP_LIBS JsonCpp::JsonCpp)
        set(AIAUDIO_HAVE_JSONCPP ON)
    endif()
endif()

# Include directories
include_directories(include)
include_directories(third_party)
if(JSONCPP_INCLUDE_DIRS)
    include_directories(${JSONCPP_INCLUDE_DIRS})
endif()

# Add subdirectories
add_subdirectory(src)

# Tests are optional - only build if GTest is available
find_package(GTest QUIET)
if(GTest_FOUND)
    add_subdirectory(tests)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-O3 -ffast-math -Wall -Wextra -Wpedantic)
    # -march=native can break portability on some CI; leave it to targets in src
elseif(MSVC)
    add_compile_options(/W4)
endif()

# Feature macros
if(AIAUDIO_HAVE_JSONCPP)
    add_compile_definitions(AIAUDIO_ENABLE_JSONCPP)
endif()