#pragma once

#include <d3d12.h>

class FDevice;
class FRootSignature;

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
    static bool Initialize(FDevice* pDevice);
    static void Shutdown();

    static ID3D12RootSignature* GetGraphicsRootSignature();
    static ID3D12RootSignature* GetComputeRootSignature();

private:
    static bool BuildRootSignature(FDevice* pDevice, FRootSignature& OutRootSignature);

    static FRootSignature mGraphicsRootSignature;
    static FRootSignature mComputeRootSignature;
    static bool mbInitialized;
};