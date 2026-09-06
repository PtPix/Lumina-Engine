cmake_minimum_required(VERSION 3.20)
project(LuminaRHI)

# ---- Public Headers ----
set(RHI_PUBLIC_HEADERS
        # Backend
        "include/D3D12Backend.h"

        # D3D12 Core
        "include/D3D12Device.h"
        "include/D3D12CommandQueue.h"
        "include/D3D12SwapChain.h"
        "include/D3D12CommandContext.h"
        "include/D3D12DeferredReleaseQueue.h"
        "include/D3D12Common.h"

        # D3D12 Descriptors
        "include/D3D12DescriptorAllocation.h"
        "include/D3D12DescriptorAllocatorPage.h"
        "include/D3D12DescriptorAllocator.h"
        "include/D3D12BindlessDescriptorHeap.h"

        # D3D12 Pipeline
        "include/D3D12Shader.h"
        "include/D3D12ShaderCompiler.h"
        "include/D3D12RootSignature.h"
        "include/D3D12PipelineState.h"
        "include/D3D12FormatUtils.h"

        # D3D12 Resource
        "include/D3D12GpuResource.h"
        "include/D3D12Buffer.h"
        "include/D3D12ResourceUploader.h"
        "include/D3D12Texture.h"
)

# ---- Private Sources ----
set(RHI_SOURCES
        # Backend
        "src/D3D12Backend.cpp"

        # D3D12 Core
        "src/D3D12Device.cpp"
        "src/D3D12CommandQueue.cpp"
        "src/D3D12SwapChain.cpp"
        "src/D3D12CommandContext.cpp"
        "src/D3D12DeferredReleaseQueue.cpp"

        # D3D12 Descriptors
        "src/D3D12DescriptorAllocation.cpp"
        "src/D3D12DescriptorAllocatorPage.cpp"
        "src/D3D12DescriptorAllocator.cpp"
        "src/D3D12BindlessDescriptorHeap.cpp"

        # D3D12 Pipeline
        "src/D3D12Shader.cpp"
        "src/D3D12ShaderCompiler.cpp"
        "src/D3D12RootSignature.cpp"
        "src/D3D12PipelineState.cpp"

        # D3D12 Resources
        "src/D3D12GpuResource.cpp"
        "src/D3D12Buffer.cpp"
        "src/D3D12ResourceUploader.cpp"
        "src/D3D12Texture.cpp"
        "src/D3D12FormatUtils.cpp"
)

# Build Library
add_library(LuminaRHI STATIC
        ${RHI_PUBLIC_HEADERS}
        ${RHI_SOURCES}
)

target_compile_features(LuminaRHI PUBLIC cxx_std_20)
target_include_directories(LuminaRHI
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Dependencies
target_link_libraries(LuminaRHI PRIVATE LuminaCore)
target_link_libraries(LuminaRHI PUBLIC D3D12MAd)
target_link_libraries(LuminaRHI PUBLIC DXC)

target_link_libraries(LuminaRHI PRIVATE
        d3d12
        dxgi
        dxguid
        d3dcompiler
)

target_link_libraries(LuminaRHI PRIVATE
        user32
        kernel32
        winmm
)

# PCH - Disabled
# target_precompile_headers(LuminaRHI PRIVATE
#         <Logger/Logger.h>
#         <Profiler/Profiler.h>
#         <StringUtils/StringConv.h>
# )

set_target_properties(LuminaRHI PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)