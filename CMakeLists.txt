cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)

# Include directories
include_directories(include)
include_directories(third_party)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(tests)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Link libraries
target_link_libraries(aiaudio_generator 
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
    pthread
    m
)