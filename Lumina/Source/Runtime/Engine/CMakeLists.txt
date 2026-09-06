cmake_minimum_required(VERSION 3.20)
project(LuminaEngine)

# ---- Public Headers ----
set(ENGINE_PUBLIC_HEADERS
        # Engine Core
        "include/Engine/LuminaEngine.h"
        "include/Engine/LuminaApp.h"
        "include/Engine/Camera.h"
        "include/Engine/Input.h"
        "include/Engine/Settings.h"

        # World
        "include/Engine/World/FScene.h"

        # Assets
        "include/Assets/StaticModel.h"
)

# ---- Private Sources ----
set(ENGINE_SOURCES
        # Engine Core
        "src/LuminaEngineMain.cpp"
        "src/LuminaApp.cpp"
        "src/Camera.cpp"
        "src/Input.cpp"

        # World
        "src/World/FScene.cpp"

        # Assets
        "src/StaticModel.cpp"
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

# PCH
# Disabled temporarily due to build issues
# target_precompile_headers(LuminaEngine PRIVATE
#         <Logger/Logger.h>
#         <Profiler/Profiler.h>
# )

set_target_properties(LuminaEngine PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
