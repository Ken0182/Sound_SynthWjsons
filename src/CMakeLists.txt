# Source files CMakeLists.txt

# Check which source files exist
set(CORE_SOURCES)
foreach(src IN ITEMS moo_optimization.cpp dsp_ir.cpp normalization.cpp 
                     semantic_fusion.cpp roles_policies.cpp decision_heads.cpp 
                     main_app.cpp audio_safety.cpp)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
        list(APPEND CORE_SOURCES ${src})
    else()
        message(STATUS "Source file ${src} not found - skipping")
    endif()
endforeach()

# Create the main library
add_library(aiaudio_core ${CORE_SOURCES})

# Set C++ standard
target_compile_features(aiaudio_core PUBLIC cxx_std_20)

# Include directories
target_include_directories(aiaudio_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link libraries
target_link_libraries(aiaudio_core PUBLIC nlohmann_json::nlohmann_json)

# Platform-specific settings
if(WIN32)
    # Windows-specific
    if(MINGW)
        target_compile_options(aiaudio_core PRIVATE -Wall -Wextra)
        target_compile_definitions(aiaudio_core PRIVATE _WIN32_WINNT=0x0601)
    elseif(MSVC)
        target_compile_options(aiaudio_core PRIVATE /W4 /permissive-)
        target_compile_definitions(aiaudio_core PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
elseif(UNIX)
    # Unix-like systems (Linux, macOS)
    target_compile_options(aiaudio_core PRIVATE -Wall -Wextra -Wpedantic)
    if(NOT APPLE)
        target_link_libraries(aiaudio_core PUBLIC pthread m)
    else()
        target_link_libraries(aiaudio_core PUBLIC pthread)
    endif()
endif()

# Optimization flags for Release builds (portable)
target_compile_options(aiaudio_core PRIVATE
    $<$<CONFIG:Release>:$<IF:$<CXX_COMPILER_ID:MSVC>,/O2,-O3>>
    $<$<CONFIG:Debug>:$<IF:$<CXX_COMPILER_ID:MSVC>,/Od,-O0>>
)

# Optional: Enable fast-math for non-strict IEEE compliance (portable)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(aiaudio_core PRIVATE 
        $<$<CONFIG:Release>:-ffast-math>
    )
endif()

# OpenCV support
if(HAVE_OPENCV)
    target_link_libraries(aiaudio_core PUBLIC ${OpenCV_LIBS})
    target_compile_definitions(aiaudio_core PUBLIC HAVE_OPENCV=1)
else()
    target_compile_definitions(aiaudio_core PUBLIC HAVE_OPENCV=0)
endif()

# Create the main executable
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    add_executable(aiaudio_generator main.cpp)
    target_link_libraries(aiaudio_generator PRIVATE aiaudio_core)
    target_compile_features(aiaudio_generator PRIVATE cxx_std_20)
else()
    message(STATUS "main.cpp not found - skipping executable target")
endif()