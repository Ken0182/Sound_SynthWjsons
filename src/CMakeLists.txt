# AI Synthesizer Source CMakeLists.txt

# Create the core library
add_library(aiaudio_core STATIC
    dsp_ir.cpp
    main_app.cpp
)

# Set target properties
target_include_directories(aiaudio_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# Link libraries
target_link_libraries(aiaudio_core PUBLIC
    ${YAMLCPP_LIBRARIES}
    Threads::Threads
)

# Create the main executable
add_executable(aiaudio_generator
    main.cpp
)

# Link the core library
target_link_libraries(aiaudio_generator PRIVATE
    aiaudio_core
)

# Set target properties
target_include_directories(aiaudio_generator PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# Platform-specific linking
if(WIN32)
    if(MINGW)
        target_link_libraries(aiaudio_generator PRIVATE m)
    endif()
elseif(UNIX AND NOT APPLE)
    target_link_libraries(aiaudio_generator PRIVATE m)
endif()

# Set output directories
set_target_properties(aiaudio_core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

set_target_properties(aiaudio_generator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Compiler-specific options
if(MSVC)
    target_compile_options(aiaudio_core PRIVATE /W4)
    target_compile_options(aiaudio_generator PRIVATE /W4)
else()
    target_compile_options(aiaudio_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(aiaudio_generator PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(aiaudio_core PRIVATE DEBUG=1)
    target_compile_definitions(aiaudio_generator PRIVATE DEBUG=1)
endif()