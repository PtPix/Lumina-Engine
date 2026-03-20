#pragma once
#include <cstdint>

enum EShaderStageFlags : uint32_t
{
    SHADER_STAGE_NONE = 0x00000000,
    SHADER_STAGE_VERTEX = 0x00000001,
    SHADER_STAGE_GEOMETRY = 0x00000002,
    SHADER_STAGE_DOMAIN = 0x00000004,
    SHADER_STAGE_HULL = 0x00000008,
    SHADER_STAGE_PIXEL = 0x00000010,
    SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
    SHADER_STAGE_COMPUTE = 0x00000020,

    SHADER_STAGE_COUNT = 6
};

enum EShaderStage : uint32_t
{
    VertexShader = 0,
    GeometryShader,
    DomainShader,
    HullShader,
    PixelShader,
    ComputeShader,

    NUM_SHADER_STAGES,
    UNINITIALIZED = NUM_SHADER_STAGES
};

enum EShaderModel : uint32_t
{
    SM5_0 = 0 , // DX11 Tessellation, compute shaders - DXBC
    SM6_0,      // DX12 DXIL
    SM6_1,      // Conservative rasterization, ROVs
    SM6_2,      // Wave intrinsics, quad ops
    SM6_3,      // DXR 1.0
    SM6_4,      // VRS, enhanced ray tracing
    SM6_5,      // Mesh shaders, task (amplification) shaders
    SM6_6,      // Atomics, dynamic resources
    SM6_7,      // DXR 1.1
    SM6_8,      // Work graphs

    NUM_SHADER_MODELS,
};

struct FShaderMacro
{
    char Name[256];
    char Value[128];
    static FShaderMacro CreateShaderMacro(const char* Name, const char* Format, ...);
};