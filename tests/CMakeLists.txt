# Tests CMakeLists.txt

# Find Google Test
find_package(GTest REQUIRED)

# Create test executable
add_executable(aiaudio_tests
    test_system.cpp
    ../src/moo_optimization.cpp
    ../src/dsp_ir.cpp
    ../src/normalization.cpp
    ../src/semantic_fusion.cpp
    ../src/roles_policies.cpp
    ../src/decision_heads.cpp
    ../src/audio_safety.cpp
    ../src/main_app.cpp
)

# Link libraries
target_link_libraries(aiaudio_tests
    GTest::gtest
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    yaml-cpp::yaml-cpp
)

# Add OpenCV if available
if(OpenCV_FOUND)
    target_link_libraries(aiaudio_tests ${OpenCV_LIBS})
    target_include_directories(aiaudio_tests PRIVATE ${OpenCV_INCLUDE_DIRS})
endif()

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(aiaudio_tests pthread m)
elseif(APPLE)
    target_link_libraries(aiaudio_tests pthread m)
endif()

# Include directories
target_include_directories(aiaudio_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Set C++ standard
set_target_properties(aiaudio_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Compiler flags
target_compile_options(aiaudio_tests PRIVATE
    -Wall -Wextra -Wpedantic
    -O2 -g
)

# Add test to CTest
enable_testing()
add_test(NAME aiaudio_tests COMMAND aiaudio_tests)