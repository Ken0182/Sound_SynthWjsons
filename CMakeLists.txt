cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(yaml-cpp QUIET)
find_package(Threads REQUIRED)
if(yaml-cpp_FOUND)
    set(YAMLCPP_LIBRARIES yaml-cpp)
    set(YAMLCPP_INCLUDE_DIRS ${yaml-cpp_INCLUDE_DIRS})
    set(YAMLCPP_FOUND TRUE)
else()
    pkg_check_modules(YAMLCPP yaml-cpp)
    if(YAMLCPP_FOUND)
        set(YAMLCPP_FOUND TRUE)
    else()
        message(WARNING "yaml-cpp not found - some features may be disabled")
        set(YAMLCPP_FOUND FALSE)
    endif()
endif()

# Find jsoncpp
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
if(NOT JSONCPP_FOUND)
    # Try to find jsoncpp library directly
    find_library(JSONCPP_LIBRARIES jsoncpp)
    if(JSONCPP_LIBRARIES)
        set(JSONCPP_FOUND TRUE)
        set(JSONCPP_INCLUDE_DIRS "")
        message(STATUS "Found jsoncpp library: ${JSONCPP_LIBRARIES}")
    else()
        message(WARNING "jsoncpp not found - JSON features will be disabled")
        set(JSONCPP_FOUND FALSE)
    endif()
endif()

# Include directories
include_directories(include)
include_directories(third_party)
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(${YAMLCPP_INCLUDE_DIRS})

# Add subdirectories
add_subdirectory(src)
# Tests are optional - only build if GTest is available
find_package(GTest QUIET)
if(GTest_FOUND)
    add_subdirectory(tests)
endif()


# Compiler flags (portable)
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
    if(CMAKE_SYSTEM_PROCESSOR)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    endif()
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Link libraries (JSON library is linked in src/CMakeLists.txt)
target_link_libraries(aiaudio_generator 
    Threads::Threads
)

# Conditionally link yaml-cpp if available
if(YAMLCPP_FOUND)
    target_link_libraries(aiaudio_generator ${YAMLCPP_LIBRARIES})
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(aiaudio_generator m)
endif()