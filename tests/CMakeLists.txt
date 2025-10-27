# Tests CMakeLists.txt

# Find Google Test (optional)
find_package(GTest QUIET)

if(GTest_FOUND)
    # Create test executable
    add_executable(aiaudio_tests
        test_system.cpp
    )

    # Link libraries
    target_link_libraries(aiaudio_tests
        aiaudio_core
        GTest::gtest
        GTest::gtest_main
        pthread
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
    
    message(STATUS "GTest found - tests will be built")
else()
    message(STATUS "GTest not found - tests will be skipped")
endif()