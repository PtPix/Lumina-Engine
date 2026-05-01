cmake_minimum_required(VERSION 3.20)
project(Renderer)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Cpp Files
set(D3D12CORE_SOURCES
        # Core
        "src/D3D12Core/Core/FDevice.cpp"
        "src/D3D12Core/Core/FCommandQueue.cpp"
        "src/D3D12Core/Core/FSwapChain.cpp"
        "src/D3D12Core/Core/FCommandContext.cpp"

        # Descriptors
        "src/D3D12Core/Descriptors/FDescriptorAllocation.cpp"
        "src/D3D12Core/Descriptors/FDescriptorAllocatorPage.cpp"
        "src/D3D12Core/Descriptors/FDescriptorAllocator.cpp"
        # "src/D3D12Core/Descriptors/FDynamicDescriptorHeap.cpp"

        "src/D3D12Core/D3D12Backend.cpp"
        "src/UI/UIRenderer.cpp"
        "src/Renderer.cpp"
        "src/Resources/FMesh.cpp"
        "src/D3D12Core/Descriptors/FBindlessDescriptorHeap.cpp"
)


set(RENDER_CORE_SOURCES
        "src/D3D12Core/Resource/FTexture.cpp"
        # "src/D3D12Core/GraphicsDevice.cpp"
        # "src/D3D12Core/Fence.cpp"
        # "src/D3D12Core/ResourceHeaps.cpp"
        # "src/D3D12Core/ResourceView.cpp"
        # "src/D3D12Core/FBuffer.cpp"
        # "src/D3D12Core/Texture.cpp"
        # "src/RenderCore/TextureManager.cpp"
        # "src/Rendering/SceneRenderTargets.cpp"
        "src/D3D12Core/Pipeline/Shader.cpp"
        "src/D3D12Core/Pipeline/ShaderCompiler.cpp"
        "src/D3D12Core/Pipeline/RootSignature.cpp"
        "src/D3D12Core/Pipeline/PipelineState.cpp"
        # "src/RenderCore/Material.cpp"
        # "src/RenderCore/BasePassMaterial.cpp"
        # "src/RenderCore/SkyboxMaterial.cpp"
        # "src/RenderCore/DeferredLightingMaterial.cpp"
        "src/D3D12Core/Resource/FBuffer.cpp"
        "src/D3D12Core/Resource/FResourceUploader.cpp"
)

set(RENDERING_SOURCES
        # "src/Rendering/SceneRenderTargets.cpp"
        # "src/Rendering/DeferredShadingRenderer.cpp"
        # "src/Rendering/Mesh.cpp"
)

# Build Library
add_library(Renderer STATIC ${D3D12CORE_SOURCES} ${RENDER_CORE_SOURCES} ${RENDERING_SOURCES})

# Interfaces
target_include_directories(Renderer PUBLIC
        include
)

# Dependencies
target_link_libraries(Renderer PRIVATE LuminaCore)
target_link_libraries(Renderer PUBLIC D3D12MAd)
target_link_libraries(Renderer PUBLIC DXC)
target_link_libraries(Renderer PUBLIC ImGUI)

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

# PCH. Make include shorter
target_precompile_headers(Renderer PRIVATE
        <Logger/Logger.h>
        <Profiler/Profiler.h>
        <StringUtils/StringConv.h>
)