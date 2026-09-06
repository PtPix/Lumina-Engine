cmake_minimum_required(VERSION 3.20)
project(LuminaEngine)

# ---- Public Headers ----
set(ENGINE_PUBLIC_HEADERS
        # Core Subsystem
        "include/Core/LuminaEngine.h"
        "include/Core/LuminaApp.h"
        "include/Core/Settings.h"

        # Scene Subsystem
        "include/Scene/Scene.h"

        # Camera Subsystem
        "include/Camera/Camera.h"

        # Input Subsystem
        "include/Input/Input.h"

        # Assets Subsystem
        "include/Assets/StaticModel.h"
)

# ---- Private Sources ----
set(ENGINE_SOURCES
        # Core Subsystem
        "src/Core/LuminaEngineMain.cpp"
        "src/Core/LuminaApp.cpp"

        # Scene Subsystem
        "src/Scene/Scene.cpp"

        # Camera Subsystem
        "src/Camera/Camera.cpp"

        # Input Subsystem
        "src/Input/Input.cpp"

        # Assets Subsystem
        "src/Assets/StaticModel.cpp"
)

# Build Library
add_library(LuminaEngine STATIC
        ${ENGINE_PUBLIC_HEADERS}
        ${ENGINE_SOURCES}
)

target_compile_features(LuminaEngine PUBLIC cxx_std_20)
target_include_directories(LuminaEngine
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Dependencies
target_link_libraries(LuminaEngine PUBLIC LuminaCore)
target_link_libraries(LuminaEngine PUBLIC Platform)
target_link_libraries(LuminaEngine PUBLIC LuminaRHI)
target_link_libraries(LuminaEngine PUBLIC LuminaRenderCore)
target_link_libraries(LuminaEngine PUBLIC LuminaRenderer)
target_link_libraries(LuminaEngine PRIVATE ImGUI)
target_link_libraries(LuminaEngine PRIVATE Assimp)

set_target_properties(LuminaEngine PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
