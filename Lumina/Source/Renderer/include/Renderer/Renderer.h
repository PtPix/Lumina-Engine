#pragma once

#include <windows.h>

#include "D3D12Core/RootSignature.h"
#include "Resources/FMesh.h"

class FCommandContext;

class Renderer
{
public:
    static bool Initialize(HWND Hwnd, uint32_t Width, uint32_t Height);
    static void Shutdown();

    static FCommandContext* BeginFrame();

    static void EndFrame(FCommandContext* pContext);

    static FMesh* CreateMesh(const FMeshData& CpuData);

    static FRootSignature* GetBindlessRootSignature() { return &mBindlessRootSignature; }

private:
    static void InitializeBindlessRootSignature();

    static FRootSignature mBindlessRootSignature;
};