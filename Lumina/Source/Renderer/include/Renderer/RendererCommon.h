#pragma once

#include <d3d12.h>

/**
 * @file RendererCommon.h
 * @brief Renderer-wide shared constants.
 *
 * Single source of truth for the global bindless root signature layout.
 * Both root signature construction (Renderer::InitializeBindlessRootSignature)
 * and per-pass binding (RenderSceneView / passes) must reference these slots
 * instead of hard-coded integers.
 */

// Root parameter slots of the global bindless root signature.
// The order here MUST match the order parameters are added in the builder.
enum class ERootParam : UINT
{
    PerObjectConstant = 0, // b0, space0 : root 32-bit constant (per-draw instance index)
    BindlessTable     = 1, // t0 space1 (SRV) + t0 space2 (buffers) : descriptor table
    GlobalCBV         = 2, // b1, space0 : root CBV (FGlobalPassData)
    InstanceSRV       = 3, // t0, space0 : root SRV (FInstanceData array)
    MaterialSRV       = 4, // t1, space0 : root SRV (FPBRMaterialData array)

    Count
};

// Convenience: cast to the UINT the D3D12 API expects.
constexpr UINT ToRootIndex(ERootParam Param)
{
    return static_cast<UINT>(Param);
}