cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
find_package(yaml-cpp REQUIRED)

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

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Link libraries
target_link_libraries(aiaudio_generator 
    ${JSONCPP_LIBRARIES}
    yaml-cpp
    pthread
    m
)