#pragma once

#include <DirectXMath.h>
#include <cstdint>

#define HLSL_CPU_COMPAT

#include "../../../../Shaders/Shared/SharedTypes.hlsli"

#undef HLSL_CPU_COMPAT

namespace ShaderInterop
{
    inline void SetTransposedMatrix(DirectX::XMMATRIX& Dest, const DirectX::XMMATRIX& Src)
    {
        Dest = DirectX::XMMatrixTranspose(Src);
    }
}
