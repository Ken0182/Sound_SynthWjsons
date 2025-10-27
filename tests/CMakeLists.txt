# Tests CMakeLists.txt

# Find Google Test (optional)
find_package(GTest QUIET)

if(GTest_FOUND)
    message(STATUS "Google Test found - building tests")
    
    # Create test executable
    add_executable(aiaudio_tests test_system.cpp)
    
    # Set C++ standard
    target_compile_features(aiaudio_tests PRIVATE cxx_std_20)
    
    # Link libraries
    target_link_libraries(aiaudio_tests PRIVATE
        aiaudio_core
        GTest::gtest
        GTest::gtest_main
    )
    
    # Platform-specific linking
    if(UNIX AND NOT APPLE)
        target_link_libraries(aiaudio_tests PRIVATE pthread)
    endif()
    
    # Include directories
    target_include_directories(aiaudio_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )
    
    # Compiler flags
    if(MSVC)
        target_compile_options(aiaudio_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(aiaudio_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    
    # Add test to CTest
    enable_testing()
    add_test(NAME aiaudio_tests COMMAND aiaudio_tests)
    
else()
    message(STATUS "Google Test not found - skipping tests")
endif()