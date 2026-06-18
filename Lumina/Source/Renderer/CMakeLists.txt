cmake_minimum_required(VERSION 3.20)
project(Renderer)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Cpp Files
set(D3D12CORE_SOURCES
        # Core
        "src/D3D12Core/Core/Device.cpp"
        "src/D3D12Core/Core/CommandQueue.cpp"
        "src/D3D12Core/Core/SwapChain.cpp"
        "src/D3D12Core/Core/CommandContext.cpp"

        # Descriptors
        "src/D3D12Core/Descriptors/DescriptorAllocation.cpp"
        "src/D3D12Core/Descriptors/DescriptorAllocatorPage.cpp"
        "src/D3D12Core/Descriptors/DescriptorAllocator.cpp"
        "src/D3D12Core/Descriptors/BindlessDescriptorHeap.cpp"

        # Pipeline
        "src/D3D12Core/Pipeline/Shader.cpp"
        "src/D3D12Core/Pipeline/ShaderCompiler.cpp"
        "src/D3D12Core/Pipeline/RootSignature.cpp"
        "src/D3D12Core/Pipeline/PipelineState.cpp"

        # Resource
        "src/D3D12Core/Resource/Buffer.cpp"
        "src/D3D12Core/Resource/ResourceUploader.cpp"
        "src/D3D12Core/Resource/Texture.cpp"

        # Backend
        "src/D3D12Core/D3D12Backend.cpp"
)

set(RENDER_GRAPH_SOURCES
        "src/RenderGraph/RenderGraph.cpp"
)


set(RENDER_CORE_SOURCES

        "src/UI/UIRenderer.cpp"
        "src/Renderer.cpp"
        "src/Resources/FMesh.cpp"

        "src/Managers/TextureManager.cpp"

        "src/Resources/FMaterial.cpp"
        "src/RenderPass/FBasePass.cpp"
        "src/Pipeline/ShaderManager.cpp"
        "src/Pipeline/PipelineStateCache.cpp"
)

# Build Library
add_library(Renderer STATIC ${D3D12CORE_SOURCES} ${RENDER_GRAPH_SOURCES} ${RENDER_CORE_SOURCES})

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