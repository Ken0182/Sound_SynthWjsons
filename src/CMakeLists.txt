# Source files CMakeLists.txt

# Create the main library
add_library(aiaudio_core
    moo_optimization.cpp
    dsp_ir.cpp
    normalization.cpp
    semantic_fusion.cpp
    roles_policies.cpp
    decision_heads.cpp
    main_app.cpp
)

# Link libraries
target_link_libraries(aiaudio_core
    nlohmann_json::nlohmann_json
    Threads::Threads
)

if (YAML_CPP_TARGET)
    target_link_libraries(aiaudio_core ${YAML_CPP_TARGET})
    target_compile_definitions(aiaudio_core PRIVATE AIAUDIO_HAVE_YAML=1)
endif()

# Include directories
target_include_directories(aiaudio_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Create the main executable
add_executable(aiaudio_generator main.cpp)

# Link with the core library
target_link_libraries(aiaudio_generator aiaudio_core)

# Set C++ standard
set_target_properties(aiaudio_core PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(aiaudio_generator PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Compiler flags
if (MSVC)
    target_compile_options(aiaudio_core PRIVATE /W4)
    target_compile_options(aiaudio_generator PRIVATE /W4)
else()
    target_compile_options(aiaudio_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(aiaudio_generator PRIVATE -Wall -Wextra -Wpedantic)
endif()