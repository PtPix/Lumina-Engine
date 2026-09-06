cmake_minimum_required(VERSION 3.20)
project(LuminaRenderer)

# ---- Public Headers ----
set(RENDERER_PUBLIC_HEADERS
        # Core
        "include/Renderer.h"
        "include/FFrameResource.h"

        # Resources
        "include/FMesh.h"
        "include/FMaterial.h"

        # UniformBuffers
        "include/ViewUniformBuffer.h"
        "include/PassUniformBuffer.h"
        "include/ObjectUniformBuffer.h"

        # Scene
        "include/Scene/SceneView.h"
        "include/Scene/SceneRenderer.h"
        "include/Scene/ViewInfo.h"
        "include/Scene/MeshDrawCommand.h"

        # Passes
        "include/Passes/RenderPass.h"
        "include/Passes/FullscreenPass.h"
        "include/Passes/BasePass/BasePass.h"
        "include/Passes/DebugPasses/DebugVisualizePass.h"

        # UI
        "include/UIRenderer.h"
)

# ---- Private Sources ----
set(RENDERER_SOURCES
        # Core
        "src/Renderer.cpp"
        "src/FFrameResource.cpp"

        # Resources
        "src/FMesh.cpp"
        "src/FMaterial.cpp"

        # UniformBuffers
        "src/ViewUniformBuffer.cpp"
        "src/ViewInfo.cpp"

        # Scene
        "src/Scene/SceneView.cpp"
        "src/SceneRenderer.cpp"

        # Passes
        "src/Passes/RenderPass.cpp"
        "src/Passes/FullscreenPass.cpp"
        "src/Passes/BasePass/BasePass.cpp"
        "src/Passes/DebugPasses/DebugVisualizePass.cpp"

        # UI
        "src/UIRenderer.cpp"
)

# Build Library
add_library(LuminaRenderer STATIC
        ${RENDERER_PUBLIC_HEADERS}
        ${RENDERER_SOURCES}
)

target_compile_features(LuminaRenderer PUBLIC cxx_std_20)
target_include_directories(LuminaRenderer
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Dependencies
target_link_libraries(LuminaRenderer PUBLIC LuminaCore)
target_link_libraries(LuminaRenderer PUBLIC LuminaRHI)
target_link_libraries(LuminaRenderer PUBLIC LuminaRenderCore)
target_link_libraries(LuminaRenderer PUBLIC LuminaEngine)  # Added: Access Engine types
target_link_libraries(LuminaRenderer PRIVATE ImGUI)

# PCH
# Disabled temporarily due to build issues
# target_precompile_headers(LuminaRenderer PRIVATE
#         <Logger/Logger.h>
#         <Profiler/Profiler.h>
# )

set_target_properties(LuminaRenderer PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
