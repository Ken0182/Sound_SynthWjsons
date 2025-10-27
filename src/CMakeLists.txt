# Source files CMakeLists.txt

# Create the main library (minimal version)
add_library(aiaudio_core
    core_types.cpp
    dsp_ir.cpp
)

# Include directories
target_include_directories(aiaudio_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(aiaudio_core
    pthread
    m
)

# Link OpenCV if available
if(OpenCV_FOUND)
    target_link_libraries(aiaudio_core ${OpenCV_LIBS})
    target_compile_definitions(aiaudio_core PRIVATE HAVE_OPENCV)
endif()

# Link yaml-cpp if available
if(yaml-cpp_FOUND)
    target_link_libraries(aiaudio_core yaml-cpp::yaml-cpp)
    target_compile_definitions(aiaudio_core PRIVATE HAVE_YAML_CPP)
endif()

# Set C++ standard
set_target_properties(aiaudio_core PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Create the main executable
add_executable(aiaudio_generator main.cpp)

# Link with the core library
target_link_libraries(aiaudio_generator aiaudio_core)

# Set C++ standard
set_target_properties(aiaudio_generator PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Platform-specific linking
if(WIN32)
    target_link_libraries(aiaudio_generator ws2_32)
endif()

# Compiler definitions
target_compile_definitions(aiaudio_core PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_definitions(aiaudio_generator PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)