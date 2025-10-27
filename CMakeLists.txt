cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -ffast-math")
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# Find OpenCV (optional for now)
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
else()
    message(WARNING "OpenCV not found - some features may be disabled")
endif()

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Find yaml-cpp
find_package(yaml-cpp REQUIRED)

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Create the main executable with all source files
add_executable(aiaudio_generator 
    src/main.cpp
    src/moo_optimization.cpp
    src/dsp_ir.cpp
    src/normalization.cpp
    src/semantic_fusion.cpp
    src/roles_policies.cpp
    src/decision_heads.cpp
    src/audio_safety.cpp
    src/main_app.cpp
)

# Link libraries
target_link_libraries(aiaudio_generator 
    nlohmann_json::nlohmann_json
    yaml-cpp::yaml-cpp
)

# Add OpenCV if available
if(OpenCV_FOUND)
    target_link_libraries(aiaudio_generator ${OpenCV_LIBS})
    target_include_directories(aiaudio_generator PRIVATE ${OpenCV_INCLUDE_DIRS})
endif()

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(aiaudio_generator pthread m)
elseif(APPLE)
    target_link_libraries(aiaudio_generator pthread m)
endif()

# Set target properties
set_target_properties(aiaudio_generator PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Include directories for the executable
target_include_directories(aiaudio_generator PRIVATE include)