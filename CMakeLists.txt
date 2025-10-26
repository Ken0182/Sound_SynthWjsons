cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator VERSION 1.0.0 LANGUAGES CXX)

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
# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    if(MSYS2)
        set(PLATFORM_MSYS2 TRUE)
    endif()
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
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
# Find pybind11 for Python bindings
find_package(pybind11 REQUIRED)

# Include directories
include_directories(include)
include_directories(third_party)
include_directories(${JSONCPP_INCLUDE_DIRS})

# Platform-specific settings
if(PLATFORM_WINDOWS)
    # Windows-specific settings
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    
    # MSYS2 specific settings
    if(PLATFORM_MSYS2)
        add_definitions(-DMSYS2_BUILD)
        # Ensure we use the correct Python
        find_package(Python3 COMPONENTS Interpreter Development)
        if(Python3_FOUND)
            set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
        endif()
    endif()
elseif(PLATFORM_LINUX)
    # Linux-specific settings
    add_definitions(-DLINUX_BUILD)
elseif(PLATFORM_MACOS)
    # macOS-specific settings
    add_definitions(-DMACOS_BUILD)
endif()

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
# Create the core library first
add_library(aiaudio_core STATIC
    src/audio_safety.cpp
    src/decision_heads.cpp
    src/dsp_ir.cpp
    src/main_app.cpp
    src/moo_optimization.cpp
    src/normalization.cpp
    src/roles_policies.cpp
    src/semantic_fusion.cpp
    src/preset_parser.cpp
)

# Link core library dependencies
target_link_libraries(aiaudio_core
    ${JSONCPP_LIBRARIES}
    yaml-cpp
    pthread
    m
)

# Python bindings target
pybind11_add_module(aiaudio_python
    src/python_bindings.cpp
)

# Compiler flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(PLATFORM_WINDOWS)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
        # Only add march=native on Linux and macOS
        if(PLATFORM_LINUX OR PLATFORM_MACOS)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -ffast-math")
        endif()
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(PLATFORM_WINDOWS)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /DEBUG")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
    endif()
endif()

# Platform-specific libraries
set(PLATFORM_LIBS "")
if(PLATFORM_LINUX OR PLATFORM_MACOS)
    list(APPEND PLATFORM_LIBS pthread m)
elseif(PLATFORM_WINDOWS)
    # Windows libraries are linked automatically
endif()

# Link libraries for main executable
target_link_libraries(aiaudio_generator 
    aiaudio_core
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
    ${PLATFORM_LIBS}
)

# Link libraries for Python module
target_link_libraries(aiaudio_python PRIVATE
    aiaudio_core
    ${JSONCPP_LIBRARIES}
    yaml-cpp
    ${PLATFORM_LIBS}
)

# Platform-specific Python module settings
if(PLATFORM_WINDOWS)
    # Windows-specific Python module settings
    set_target_properties(aiaudio_python PROPERTIES
        SUFFIX ".pyd"
        PREFIX ""
    )
else()
    # Unix-specific Python module settings
    set_target_properties(aiaudio_python PROPERTIES
        SUFFIX ".so"
        PREFIX ""
    )
endif()