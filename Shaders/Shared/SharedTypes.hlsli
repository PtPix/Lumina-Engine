// @file SharedTypes.hlsli
// @brief CPU / GPU shared types and constants

#ifndef SHARED_TYPES_HLSLI
#define SHARED_TYPES_HLSLI

// Type
#ifdef HLSL_CPU_COMPAT
    #define float2 DirectX::XMFLOAT2
    #define float3 DirectX::XMFLOAT3
    #define float4 DirectX::XMFLOAT4
    #define float4x4 DirectX::XMMATRIX
    #define uint uint32_t
    #define uint2 DirectX::XMUINT2
    #define uint3 DirectX::XMUINT3
    #define uint4 DirectX::XMUINT4
#endif

// View Uniform (Per View)
#ifdef HLSL_CPU_COMPAT
struct alignas(16) FViewUniform
#else
struct FViewUniform
#endif
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
    float4x4 ViewProjectionMatrix;

    float4x4 InvViewMatrix;
    float4x4 InvProjectionMatrix;
    float4x4 InvViewProjectionMatrix;

    float4x4 PrevViewProjectionMatrix;

    float3 CameraPosition;
    float NearPlane;

    float3 CameraDirection;
    float FarPlane;

    float2 ViewportSize;     // (Width, Height)
    float2 ViewportSizeRcp;  // (1 / Width, 1 / Height)

    float2 Jitter;     // TAA subpixel jitter (range: [-0.5, 0.5])
    uint FrameIndex;
    uint _ViewPad1;
};

// Instance Data (Per Instance)
#ifdef HLSL_CPU_COMPAT
struct alignas(16) FInstanceData
#else
struct FInstanceData
#endif
{
    float4x4 WorldMatrix;
    uint MaterialIndex;
    uint _InstancePad[3];
};

// PBR Material Data
#ifdef HLSL_CPU_COMPAT
struct alignas(16) FPBRMaterialData
#else
struct FPBRMaterialData
#endif
{
    float4 BaseColorFactor;
    float4 EmissiveFactor;

    float  MetallicFactor;
    float  RoughnessFactor;
    float  AlphaCutoff;
    uint   ShadingModel;

    uint   AlbedoTexIndex;
    uint   NormalTexIndex;
    uint   ORMTexIndex;
    uint   EmissiveTexIndex;

    uint   MaterialFlags;
    uint   _MaterialPad[3];
};

// Light Data
#ifdef HLSL_CPU_COMPAT
struct alignas(16) FDirectionalLight
#else
struct FDirectionalLight
#endif
{
    float3 Direction;
    float Intensity;

    float3 Color;
    uint _LightPad1;
};

#ifdef HLSL_CPU_COMPAT
    #undef float2
    #undef float3
    #undef float4
    #undef float4x4
    #undef uint
    #undef uint2
    #undef uint3
    #undef uint4
#endif

#endif