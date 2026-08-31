/**
  * @file ObjectUniformBuffer.h
  * @brief Object-level uniform buffer.
  *
  * Contains per-object constants: transform matrix, material ID, etc.
  * For GPU-driven rendering, this might be replaced by StructuredBuffer.
  */

#pragma once

#include "UniformBuffer.h"
#include <DirectXMath.h>

struct alignas(256) FObjectUniformData
{
    DirectX::XMMATRIX WorldMatrix;
    DirectX::XMMATRIX WorldMatrixInverseTranspose;

    uint32_t MaterialIndex;
    uint32_t InstanceID;
    uint32_t Padding0;
    uint32_t Padding1;
};

static_assert(sizeof(FObjectUniformData) == 256, "Must be 256 bytes");

using FObjectUniformBuffer = TUniformBuffer<FObjectUniformData>;