cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific configuration
if(WIN32)
    # Windows/MSYS2 specific settings
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif(UNIX AND NOT APPLE)
    # Linux specific settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
elseif(APPLE)
    # macOS specific settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework CoreAudio")
endif()

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(yaml-cpp REQUIRED)

# Find jsoncpp
find_package(jsoncpp REQUIRED)
if(jsoncpp_FOUND)
    set(JSONCPP_LIBRARIES jsoncpp_lib)
    set(JSONCPP_INCLUDE_DIRS ${jsoncpp_INCLUDE_DIRS})
else()
    # Fallback to pkg-config
    pkg_check_modules(JSONCPP REQUIRED jsoncpp)
endif()

# Find Python and pybind11 for Python bindings
find_package(Python3 COMPONENTS Interpreter Development QUIET)
find_package(pybind11 QUIET)

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

# Python bindings are optional - only build if pybind11 is available
if(pybind11_FOUND AND Python3_FOUND)
    message(STATUS "Building Python bindings with pybind11")
    pybind11_add_module(aiaudio_cpp src/python_bindings.cpp)
    
    target_link_libraries(aiaudio_cpp PRIVATE
        aiaudio_core
        ${JSONCPP_LIBRARIES}
        yaml-cpp
    )
    
    if(UNIX AND NOT APPLE)
        target_link_libraries(aiaudio_cpp PRIVATE pthread m)
    endif()
    
    # Set output directory for Python module
    set_target_properties(aiaudio_cpp PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
else()
    message(WARNING "pybind11 or Python3 not found - skipping Python bindings")
    message(WARNING "Install with: pip install pybind11")
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
    # Only use -march=native if not cross-compiling
    if(NOT CMAKE_CROSSCOMPILING)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    endif()
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Link libraries
target_link_libraries(aiaudio_generator 
    ${JSONCPP_LIBRARIES}
    yaml-cpp
)

if(UNIX AND NOT APPLE)
    target_link_libraries(aiaudio_generator pthread m)
endif()

# Installation rules
install(TARGETS aiaudio_generator
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

if(pybind11_FOUND AND Python3_FOUND)
    install(TARGETS aiaudio_cpp
        LIBRARY DESTINATION ${Python3_SITELIB}
    )
endif()