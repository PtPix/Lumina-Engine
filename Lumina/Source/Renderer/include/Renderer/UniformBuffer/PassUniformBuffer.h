/**
  * @file PassUniformBuffer.h
  * @brief Pass-level uniform buffer.
  *
  * Contains per-pass constants that are shared across all draws in a pass.
  * Examples: shadow map indices, lighting parameters, pass-specific flags.
  */

#pragma once

#include "UniformBuffer.h"
#include <DirectXMath.h>

struct alignas(256) FBasePassUniformData
{
    uint32_t MaterialBufferIndex;
    uint32_t InstanceBufferIndex;
    uint32_t Padding0;
    uint32_t Padding1;

    // Add more as needed
};

static_assert(sizeof(FBasePassUniformData) == 256, "Must be 256 bytes");

using FBasePassUniformBuffer = TUniformBuffer<FBasePassUniformData>;

struct alignas(256) FShadowPassUniformData
{
    DirectX::XMMATRIX LightViewProjectionMatrix;

    uint32_t CascadeIndex;
    float DepthBias;
    float SlopeScaledDepthBias;
    uint32_t Padding0;

    // Add more as needed
};

static_assert(sizeof(FShadowPassUniformData) == 256, "Must be 256 bytes");

using FShadowPassUniformBuffer = TUniformBuffer<FShadowPassUniformData>;