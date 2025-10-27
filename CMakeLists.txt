cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Find nlohmann/json (try system package first, fallback to bundled)
find_package(nlohmann_json 3.2.0 QUIET)

if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found in system, checking for bundled header...")
    
    # Check for bundled single-header version
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nlohmann/json.hpp")
        message(STATUS "Using bundled nlohmann/json from third_party/")
        add_library(nlohmann_json INTERFACE)
        target_include_directories(nlohmann_json INTERFACE 
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/third_party>
            $<INSTALL_INTERFACE:include>
        )
        add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
    else()
        message(FATAL_ERROR 
            "nlohmann/json not found!\n"
            "Please either:\n"
            "  1. Install system package: apt-get install nlohmann-json3-dev (Ubuntu)\n"
            "                            brew install nlohmann-json (macOS)\n"
            "                            pacman -S mingw-w64-x86_64-nlohmann-json (MSYS2)\n"
            "  2. Download single header to third_party/nlohmann/json.hpp:\n"
            "     mkdir -p third_party/nlohmann\n"
            "     curl -o third_party/nlohmann/json.hpp \\\n"
            "       https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp\n"
        )
    endif()
else()
    message(STATUS "Found system nlohmann_json: ${nlohmann_json_VERSION}")
endif()

# Optional: OpenCV (if available, otherwise disable features that need it)
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    set(HAVE_OPENCV 1)
else()
    message(STATUS "OpenCV not found - some features will be disabled")
    set(HAVE_OPENCV 0)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Platform-specific linking
if(WIN32)
    # Windows-specific libraries
    target_link_libraries(aiaudio_generator PRIVATE nlohmann_json::nlohmann_json)
    if(HAVE_OPENCV)
        target_link_libraries(aiaudio_generator PRIVATE ${OpenCV_LIBS})
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux-specific libraries
    target_link_libraries(aiaudio_generator PRIVATE 
        nlohmann_json::nlohmann_json
        pthread
        m
    )
    if(HAVE_OPENCV)
        target_link_libraries(aiaudio_generator PRIVATE ${OpenCV_LIBS})
    endif()
elseif(APPLE)
    # macOS-specific libraries
    target_link_libraries(aiaudio_generator PRIVATE 
        nlohmann_json::nlohmann_json
        pthread
    )
    if(HAVE_OPENCV)
        target_link_libraries(aiaudio_generator PRIVATE ${OpenCV_LIBS})
    endif()
endif()