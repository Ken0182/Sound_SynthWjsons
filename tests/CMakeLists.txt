# AI Synthesizer Tests CMakeLists.txt

# Find required packages for testing
find_package(Threads REQUIRED)

# Create test executable
add_executable(test_system
    test_system.cpp
)

# Link libraries
target_link_libraries(test_system PRIVATE
    aiaudio_core
    Threads::Threads
)

# Set target properties
target_include_directories(test_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# Platform-specific linking
if(WIN32)
    if(MINGW)
        target_link_libraries(test_system PRIVATE m)
    endif()
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_system PRIVATE m)
endif()

# Set output directory
set_target_properties(test_system PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Compiler-specific options
if(MSVC)
    target_compile_options(test_system PRIVATE /W4)
else()
    target_compile_options(test_system PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Add test to CTest
add_test(NAME system_tests COMMAND test_system)

# Set test properties
set_tests_properties(system_tests PROPERTIES
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)