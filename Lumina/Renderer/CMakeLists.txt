cmake_minimum_required(VERSION 3.20)
project(Renderer)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Cpp Files
set(D3D12CORE_SOURCES
        "src/D3D12Core/Device.cpp"
        "src/D3D12Core/CommandQueue.cpp"
        "src/D3D12Core/Fence.cpp"
        "src/D3D12Core/SwapChain.cpp"
        "src/D3D12Core/GraphicsDevice.cpp"
)

set(RENDER_CORE_SOURCES
        "src/RenderCore/ResourceHeaps.cpp"
        "src/RenderCore/ResourceView.cpp"
        "src/RenderCore/Buffer.cpp"
        "src/RenderCore/Texture.cpp"
        "src/RenderCore/TextureManager.cpp"
        "src/Rendering/SceneRenderTargets.cpp"
        "src/RenderCore/Shader.cpp"
        "src/RenderCore/ShaderCompiler.cpp"
        "src/RenderCore/RootSignature.cpp"
        "src/RenderCore/PipelineState.cpp"
        "src/RenderCore/Material.cpp"
        "src/RenderCore/BasePassMaterial.cpp"
        "src/RenderCore/SkyboxMaterial.cpp"
        "src/RenderCore/DeferredLightingMaterial.cpp"
)

set(RENDERING_SOURCES
        "src/Rendering/SceneRenderTargets.cpp"
        "src/Rendering/DeferredShadingRenderer.cpp"
        "src/Rendering/Mesh.cpp"
)

# Build Library
add_library(Renderer STATIC ${D3D12CORE_SOURCES} ${RENDER_CORE_SOURCES} ${RENDERING_SOURCES})

# Interfaces
target_include_directories(Renderer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Dependencies
target_link_libraries(Renderer PRIVATE Logger)
target_link_libraries(Renderer PUBLIC D3D12MAd)
target_link_libraries(Renderer PUBLIC DXC)

target_link_libraries(Renderer PRIVATE
    d3d12.lib
    dxgi.lib
    dxguid.lib
    d3dcompiler.lib
)

target_link_libraries(Renderer PRIVATE
    user32.lib
    kernel32.lib
    winmm.lib
)