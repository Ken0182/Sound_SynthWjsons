cmake_minimum_required(VERSION 3.20)
project(AISynthesizer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Find required packages
find_package(PkgConfig QUIET)
find_package(Threads REQUIRED)

# Find yaml-cpp with proper target
find_package(yaml-cpp QUIET)
if(yaml-cpp_FOUND)
    set(YAMLCPP_LIBRARIES yaml-cpp::yaml-cpp)
    set(YAMLCPP_INCLUDE_DIRS ${yaml-cpp_INCLUDE_DIRS})
    message(STATUS "Found yaml-cpp: ${yaml-cpp_VERSION}")
else()
    # Fallback to pkg-config
    if(PkgConfig_FOUND)
        pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
        set(YAMLCPP_LIBRARIES ${YAMLCPP_LIBRARIES})
        set(YAMLCPP_INCLUDE_DIRS ${YAMLCPP_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "yaml-cpp not found. Please install yaml-cpp development package.")
    endif()
endif()

# Include directories
include_directories(include)
include_directories(third_party)
include_directories(${YAMLCPP_INCLUDE_DIRS})

# Add subdirectories
add_subdirectory(src)

# Tests
enable_testing()
add_subdirectory(tests)

# CPack configuration
include(CPack)

# Platform-specific linking
if(WIN32)
    target_link_libraries(aiaudio_core ${YAMLCPP_LIBRARIES} Threads::Threads)
    if(MINGW)
        target_link_libraries(aiaudio_core m)
    endif()
elseif(UNIX AND NOT APPLE)
    target_link_libraries(aiaudio_core ${YAMLCPP_LIBRARIES} Threads::Threads m)
elseif(APPLE)
    target_link_libraries(aiaudio_core ${YAMLCPP_LIBRARIES} Threads::Threads)
endif()

# Installation
install(TARGETS aiaudio_core aiaudio_generator
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)
install(FILES README.md LICENSE DESTINATION .)