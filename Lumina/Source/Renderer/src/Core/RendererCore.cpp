#include "Renderer/Core/RendererCore.h"
#include "Renderer/D3D12/D3D12Backend.h"
#include "Renderer/D3D12/D3D12Device.h"
#include "D3D12MemAlloc.h"

#include <cassert>

bool FRendererCore::mbInitialized = false;
FD3D12Backend* FRendererCore::mpBackend = nullptr;

uint32_t FRendererCore::mCurrentFrameIndex = 0;
uint64_t FRendererCore::mFrameNumber = 0;

uint32_t FRendererCore::mRenderWidth = 1920;
uint32_t FRendererCore::mRenderHeight = 1080;

FRenderStats FRendererCore::mStats = {};

bool FRendererCore::Initialize(const FRendererInitParams &Params)
{
    if (mbInitialized) return true;

    mRenderWidth = Params.Width;
    mRenderHeight = Params.Height;

    mbInitialized = true;

    return true;
}

void FRendererCore::Shutdown()
{
    if (!mbInitialized) return;

    mpBackend = nullptr;
    mbInitialized = false;
}

void FRendererCore::BeginFrame()
{
    assert(mbInitialized && "RendererCore not initialized!");

    mCurrentFrameIndex = (mCurrentFrameIndex + 1) % NUM_FRAMES_IN_FLIGHT;
    mFrameNumber++;

    mStats.Reset();
}

void FRendererCore::EndFrame()
{
    assert(mbInitialized && "RendererCore not initialized!");
}

FD3D12Device * FRendererCore::GetDevice()
{
    assert(mbInitialized && mpBackend && "RendererCore not initialized!");
    return mpBackend->GetDevice();
}

D3D12MA::Allocator * FRendererCore::GetAllocator()
{
    assert(mbInitialized && mpBackend && "RendererCore not initialized!");
    return mpBackend->GetAllocator();
}

void FRendererCore::SetRenderResolution(uint32_t Width, uint32_t Height)
{
    mRenderWidth = Width;
    mRenderHeight = Height;
}
