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
else()
    pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
endif()

# Find jsoncpp
find_package(jsoncpp REQUIRED)
if(jsoncpp_FOUND)
    set(JSONCPP_LIBRARIES jsoncpp_lib)
    set(JSONCPP_INCLUDE_DIRS ${jsoncpp_INCLUDE_DIRS})
else()
    # Fallback to pkg-config
    pkg_check_modules(JSONCPP REQUIRED jsoncpp)
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

# Link libraries
target_link_libraries(aiaudio_generator 
    ${JSONCPP_LIBRARIES}
    ${YAMLCPP_LIBRARIES}
    Threads::Threads
)

if(UNIX AND NOT APPLE)
    target_link_libraries(aiaudio_generator m)
endif()