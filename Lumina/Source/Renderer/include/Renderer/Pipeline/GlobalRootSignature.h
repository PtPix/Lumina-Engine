#pragma once

#include <d3d12.h>

class FD3D12Device;
class FD3D12RootSignature;

// Layout :
//     Slot 0: Bindless index and PerDraw data
//     Slot 1: View CBV
//     Slot 2: Bindless Table

enum class EGlobalRootParam : UINT
{
    RootConstants = 0, // RootParameter(16 * uint32)
    ViewCBV = 1, // b0, space0: FViewUniform
    BindlessTable =2, // t0 / u0 / b1 ...

    Count
};

constexpr UINT ToRootIndex(EGlobalRootParam Param)
{
    return static_cast<UINT>(Param);
}

class FGlobalRootSignature
{
public:
    static bool Initialize(FD3D12Device* pDevice);
    static void Shutdown();

    static ID3D12RootSignature* GetGraphicsRootSignature();
    static ID3D12RootSignature* GetComputeRootSignature();

private:
    static bool BuildRootSignature(FD3D12Device* pDevice, FD3D12RootSignature& OutRootSignature);

    static FD3D12RootSignature mGraphicsRootSignature;
    static FD3D12RootSignature mComputeRootSignature;
    static bool mbInitialized;
};