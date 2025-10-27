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
    Threads::Threads
)

# Conditionally link jsoncpp if available
if(JSONCPP_FOUND)
    target_link_libraries(aiaudio_core ${JSONCPP_LIBRARIES})
    target_compile_definitions(aiaudio_core PRIVATE JSONCPP_FOUND)
endif()

# Conditionally link yaml-cpp if available
if(YAMLCPP_FOUND)
    target_link_libraries(aiaudio_core ${YAMLCPP_LIBRARIES})
    target_compile_definitions(aiaudio_core PRIVATE YAMLCPP_FOUND)
endif()

# Link math library on non-Windows
if(UNIX AND NOT APPLE)
    target_link_libraries(aiaudio_core m)
endif()

# Include directories
target_include_directories(aiaudio_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${JSONCPP_INCLUDE_DIRS}
)

# Create the main executable
add_executable(aiaudio_generator main.cpp)

# Link with the core library and JSON library
target_link_libraries(aiaudio_generator aiaudio_core)

# Conditionally link jsoncpp if available
if(JSONCPP_FOUND)
    target_link_libraries(aiaudio_generator ${JSONCPP_LIBRARIES})
endif()

# Set C++ standard
set_target_properties(aiaudio_core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(aiaudio_generator PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Compiler flags
if(NOT MSVC)
    target_compile_options(aiaudio_core PRIVATE
        -Wall -Wextra -Wpedantic
        -O3 -ffast-math
    )
    # Only enable -march=native when not cross-compiling
    if(CMAKE_SYSTEM_PROCESSOR)
        target_compile_options(aiaudio_core PRIVATE -march=native)
    endif()
endif()

if(NOT MSVC)
    target_compile_options(aiaudio_generator PRIVATE
        -Wall -Wextra -Wpedantic
        -O3 -ffast-math
    )
    if(CMAKE_SYSTEM_PROCESSOR)
        target_compile_options(aiaudio_generator PRIVATE -march=native)
    endif()
endif()

# Optional: Python bindings via pybind11
find_package(pybind11 CONFIG QUIET)
if(pybind11_FOUND)
    message(STATUS "pybind11 found: enabling Python bindings")
    pybind11_add_module(aiaudio_python
        python_bindings.cpp
    )
    target_link_libraries(aiaudio_python PRIVATE aiaudio_core)
    target_include_directories(aiaudio_python PRIVATE ${CMAKE_SOURCE_DIR}/include)

    # Define for Windows DLL export quirks if needed
    if(WIN32)
        target_compile_definitions(aiaudio_python PRIVATE _USE_MATH_DEFINES)
    endif()
else()
    message(STATUS "pybind11 not found: skipping Python bindings")
endif()