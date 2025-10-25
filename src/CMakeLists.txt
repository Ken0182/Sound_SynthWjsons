# Source files CMakeLists.txt

# Create the main library
add_library(aiaudio_core
    audio_safety.cpp
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
    ${JSONCPP_LIBRARIES}
    ${YAML_CPP_LIBRARIES}
)

# Add pthread for Unix-like systems
if(UNIX)
    target_link_libraries(aiaudio_core pthread m)
endif()

# Add yaml-cpp if found as a target
if(TARGET yaml-cpp)
    target_link_libraries(aiaudio_core yaml-cpp)
endif()

# Include directories
target_include_directories(aiaudio_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${JSONCPP_INCLUDE_DIRS}
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
target_compile_options(aiaudio_core PRIVATE
    -Wall -Wextra -Wpedantic
    -O3 -march=native -ffast-math
)

target_compile_options(aiaudio_generator PRIVATE
    -Wall -Wextra -Wpedantic
    -O3 -march=native -ffast-math
)