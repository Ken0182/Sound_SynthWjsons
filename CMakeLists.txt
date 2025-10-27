cmake_minimum_required(VERSION 3.20)
project(AIAudioGenerator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
include(CTest)

# Threads (portable)
find_package(Threads REQUIRED)

# nlohmann_json: prefer system, fallback to FetchContent (header-only)
find_package(nlohmann_json CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
  message(STATUS "nlohmann_json not found; fetching via FetchContent")
  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
  )
  set(JSON_BuildTests OFF CACHE INTERNAL "")
  FetchContent_MakeAvailable(nlohmann_json)
endif()

# yaml-cpp: optional. Prefer system; disable fetch on CI lacking network.
set(YAML_CPP_TARGET "")
find_package(yaml-cpp CONFIG QUIET)
if(TARGET yaml-cpp::yaml-cpp)
  set(YAML_CPP_TARGET yaml-cpp::yaml-cpp)
elseif(TARGET yaml-cpp)
  add_library(yaml-cpp::yaml-cpp ALIAS yaml-cpp)
  set(YAML_CPP_TARGET yaml-cpp::yaml-cpp)
else()
  message(STATUS "yaml-cpp not found; building without YAML policy support. Some features disabled.")
endif()

# Provide global include directory for project headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Testing setup (ensure GTest targets exist before adding tests)
if(BUILD_TESTING)
  # Try system GTest, else FetchContent
  find_package(GTest CONFIG QUIET)
  if(NOT TARGET GTest::gtest)
    message(STATUS "GTest not found; fetching via FetchContent")
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.14.0
    )
    set(INSTALL_GTEST OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(googletest)
  endif()
endif()

# Subdirectories
add_subdirectory(src)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()