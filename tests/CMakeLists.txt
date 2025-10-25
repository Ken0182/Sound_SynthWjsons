# Tests CMakeLists.txt

# Find Google Test
find_package(GTest REQUIRED)

# Create test executable
add_executable(aiaudio_tests
    test_system.cpp
)

# Link libraries
target_link_libraries(aiaudio_tests
    aiaudio_core
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

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