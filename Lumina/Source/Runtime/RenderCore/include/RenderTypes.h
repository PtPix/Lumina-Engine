/*
 * @file RenderTypes.h
 * @brief Core rendering type definitions and constants.
 *
 * Central location for fundamental types, enums, and constants used across the renderer.
 * Avoids circular dependencies by keeping common types in one place.
 */

#pragma once

#include <cstdint>
#include <DirectXMath.h>

// Frame Configuration
constexpr uint32_t NUM_FRAMES_IN_FLIGHT = 3;
constexpr uint32_t MAX_RENDER_TARGETS = 8;

// Render Pass Types
enum class ERenderPassType : uint8_t
{
    Graphics,
    Compute,
    Copy,
    RayTracing,
};

// Primitive Types
enum class EPrimitiveType : uint8_t
{
    StaticMesh,
    SkeletalMesh,
    Terrain,
    Particle,
    Volume,
};

// Shading Models
enum class EShadingModel : uint8_t
{
    Unlit = 0,
    DefaultLit = 1,
    Subsurface = 2,
    TwoSidedFoliage = 3,
    ClearCoat = 4,
    Cloth = 5,
};

// Blend Modes
enum class EBlendMode : uint8_t
{
    Opaque,
    Masked,
    Translucent,
    Additive,
    Modulate,
    AlphaBlend, // src.a * src + (1 - src.a) * dst
    PremultipliedAlpha, // src + (1 - src.a) * dst
};

// Common Structures
struct FRenderViewport
{
    float X = 0.0f;
    float Y = 0.0f;
    float Width = 1920.0f;
    float Height = 1080.0f;
    float MinDepth = 0.0f;
    float MaxDepth = 1.0f;
};

struct FRenderRect
{
    int32_t X = 0;
    int32_t Y = 0;
    int32_t Width = 1920;
    int32_t Height = 1080;
};

// Handle Types
struct FTextureHandle
{
    uint32_t Index = UINT32_MAX;
    [[nodiscard]] bool IsValid() const { return Index != UINT32_MAX; }
};

struct FBufferHandle
{
    uint32_t Index = UINT32_MAX;
    [[nodiscard]] bool IsValid() const { return Index != UINT32_MAX; }
};

struct FMaterialHandle
{
    uint32_t Index = UINT32_MAX;
    [[nodiscard]] bool IsValid() const { return Index != UINT32_MAX; }
};

struct FMeshHandle
{
    uint32_t Index = UINT32_MAX;
    [[nodiscard]] bool IsValid() const { return Index != UINT32_MAX; }
};

// Statistics
struct FRenderStats
{
    uint32_t NumDrawCalls = 0;
    uint32_t NumTriangles = 0;
    uint32_t NumInstances = 0;
    uint32_t NumComputeDispatches = 0;

    float FrameTimeMs = 0.0f;
    float GpuTimeMs = 0.0f;

    void Reset()
    {
        NumDrawCalls = 0;
        NumTriangles = 0;
        NumInstances = 0;
        NumComputeDispatches = 0;
    }
};