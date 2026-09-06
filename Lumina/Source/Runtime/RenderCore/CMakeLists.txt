cmake_minimum_required(VERSION 3.20)
project(LuminaRenderCore)

# ---- Public Headers ----
set(RENDERCORE_PUBLIC_HEADERS
        # Core
        "include/RenderCore.h"
        "include/RenderTypes.h"
        "include/RendererModule.h"
        "include/SharedTypes.h"
        "include/MeshType.h"

        # RenderGraph
        "include/RenderGraph/RenderGraph.h"

        # UniformBuffer
        "include/UniformBuffer.h"
)

# ---- Private Sources ----
set(RENDERCORE_SOURCES
        # Core
        "src/RenderCore.cpp"
        "src/RendererModule.cpp"

        # RenderGraph
        "src/RenderGraph/RenderGraph.cpp"
)

# Build Library
add_library(LuminaRenderCore STATIC
        ${RENDERCORE_PUBLIC_HEADERS}
        ${RENDERCORE_SOURCES}
)

target_compile_features(LuminaRenderCore PUBLIC cxx_std_20)
target_include_directories(LuminaRenderCore
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Dependencies
target_link_libraries(LuminaRenderCore PUBLIC LuminaCore)
target_link_libraries(LuminaRenderCore PUBLIC LuminaRHI)

# PCH
# Disabled temporarily due to build issues
# target_precompile_headers(LuminaRenderCore PRIVATE
#         <Logger/Logger.h>
#         <Profiler/Profiler.h>
# )

set_target_properties(LuminaRenderCore PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
