#include "Renderer/GPUScene/GPUScene.h"
#include "Renderer/D3D12/D3D12Device.h"
#include "Renderer/Core/RendererCore.h"
#include "D3D12MemAlloc.h"
#include <algorithm>

bool FGPUScene::Initialize(FD3D12Device *pDevice, const FGPUSceneConfig &Config)
{
    if (mbInitialized) return true;

    if (!pDevice) return false;

    mpDevice = pDevice;
    mpAllocator = pDevice->GetAllocator();
    mConfig = Config;

    mInstanceData.reserve(mConfig.MaxInstances);
    mMaterialData.reserve(mConfig.MaxMaterials);
    mInstanceData.reserve(mConfig.MaxInstances);

    if (!CreateBuffers())
    {
        Shutdown();
        return false;
    }

    mbInitialized = true;
    return true;
}

void FGPUScene::Shutdown()
{
    if (!mbInitialized) return;

    for (uint32_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
    {
        mInstanceBuffer[i].Destroy();
        mMaterialBuffer[i].Destroy();
        mPrimitiveBuffer[i].Destroy();
    }

    mInstanceData.clear();
    mMaterialData.clear();
    mPrimitiveData.clear();
    mPrimitiveIDToIndex.clear();

    mbInitialized = false;
}

uint32_t FGPUScene::AddPrimitive(const FGPUInstanceData &InstanceData, const FGPUPrimitiveData &PrimitiveData)
{
    if (!mbInitialized) return 0;

    uint32_t PrimitiveID = mNextPrimitiveID++;

    auto InstanceIndex = static_cast<uint32_t>(mInstanceData.size());
    mInstanceData.push_back(InstanceData);

    FGPUPrimitiveData PrimData = PrimitiveData;
    PrimData.InstanceIndex = InstanceIndex;
    PrimData.PrimitiveID = PrimitiveID;

    auto PrimIndex = static_cast<uint32_t>(mPrimitiveData.size());
    mPrimitiveData.push_back(PrimData);

    mPrimitiveIDToIndex[PrimitiveID] = PrimIndex;

    mbInstanceDataDirty = true;
    mbPrimitiveDataDirty = true;

    return PrimitiveID;
}

void FGPUScene::UpdatePrimitive(uint32_t PrimitiveID, const FGPUInstanceData &InstanceData)
{
    if (!mbInitialized) return;

    auto It = mPrimitiveIDToIndex.find(PrimitiveID);
    if (It == mPrimitiveIDToIndex.end()) return;

    uint32_t PrimIndex = It->second;
    if (PrimIndex >= mInstanceData.size()) return;

    uint32_t InstanceIndex = mPrimitiveData[PrimIndex].InstanceIndex;
    if (InstanceIndex < mInstanceData.size())
    {
        mInstanceData[InstanceIndex] = InstanceData;
        mbInstanceDataDirty = true;
    }
}

void FGPUScene::RemovePrimitive(uint32_t PrimitiveID)
{
    if (!mbInitialized) return;

    auto It = mPrimitiveIDToIndex.find(PrimitiveID);
    if (It == mPrimitiveIDToIndex.end()) return;

    uint32_t PrimIndex = It->second;

    if (PrimIndex < mPrimitiveData.size() - 1)
    {
        mPrimitiveData[PrimIndex] = mPrimitiveData.back();

        uint32_t SwappedID = mPrimitiveData[PrimIndex].PrimitiveID;
        mPrimitiveIDToIndex[SwappedID] = PrimIndex;
    }

    mPrimitiveData.pop_back();
    mPrimitiveIDToIndex.erase(PrimitiveID);

    // TODO: here should be a free-list to free instance data
    mbPrimitiveDataDirty = true;
}

uint32_t FGPUScene::AddMaterial(const FGPUMaterialData &MaterialData)
{
    if (!mbInitialized) return 0;

    uint32_t MaterialIndex = mNextMaterialIndex++;

    if (MaterialIndex >= mMaterialData.size())
    {
        mMaterialData.resize(MaterialIndex + 1);
    }

    mMaterialData[MaterialIndex] = MaterialData;
    mbMaterialDataDirty = true;

    return MaterialIndex;
}

void FGPUScene::UpdateMaterial(uint32_t MaterialIndex, const FGPUMaterialData &MaterialData)
{
    if (!mbInitialized || MaterialIndex >= mMaterialData.size())
    {
        return;
    }

    mMaterialData[MaterialIndex] = MaterialData;
    mbMaterialDataDirty = true;
}

void FGPUScene::UploadToGPU(uint32_t FrameIndex)
{
    if (!mbInitialized || FrameIndex >= NUM_FRAMES_IN_FLIGHT) return;

    if (mbInstanceDataDirty)
    {
        UploadInstanceData(FrameIndex);
        mbInstanceDataDirty = false;
    }

    if (mbMaterialDataDirty)
    {
        UploadMaterialData(FrameIndex);
        mbMaterialDataDirty = false;
    }

    if (mbPrimitiveDataDirty)
    {
        UploadPrimitiveData(FrameIndex);
        mbPrimitiveDataDirty = false;
    }
}

bool FGPUScene::CreateBuffers()
{
    for (uint32_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
    {
        if (!mInstanceBuffer[i].Create(mpDevice, mpAllocator, sizeof(FGPUInstanceData), mConfig.MaxInstances,
            false, false, D3D12_HEAP_TYPE_UPLOAD, L"GPUScene_InstanceBuffer"))
        {
            return false;
        }
    }

    mInstanceBufferSRVIndex = mInstanceBuffer[0].GetBindlessSRVIndex();

    for (uint32_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
    {
        if (!mPrimitiveBuffer[i].Create(mpDevice, mpAllocator, sizeof(FGPUPrimitiveData), mConfig.MaxPrimitives,
            false, false, D3D12_HEAP_TYPE_UPLOAD, L"GPUScene_PrimitiveBuffer"))
        {
            return false;
        }
    }

    mPrimitiveBufferSRVIndex = mPrimitiveBuffer[0].GetBindlessSRVIndex();

    for (uint32_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
    {
        if (!mMaterialBuffer[i].Create(mpDevice, mpAllocator, sizeof(FGPUMaterialData), mConfig.MaxMaterials,
            false, false, D3D12_HEAP_TYPE_UPLOAD, L"GPUScene_MaterialBuffer"))
        {
            return false;
        }
    }

    mMaterialBufferSRVIndex = mMaterialBuffer[0].GetBindlessSRVIndex();

    return true;
}

void FGPUScene::UploadInstanceData(uint32_t FrameIndex)
{
    if (mInstanceData.empty()) return;

    void* MappedData = mInstanceBuffer[FrameIndex].Map();
    if (MappedData)
    {
        size_t BytesToCopy = mInstanceData.size() * sizeof(FGPUInstanceData);
        memcpy(MappedData, mInstanceData.data(), BytesToCopy);
        mInstanceBuffer[FrameIndex].Unmap();
    }
}

void FGPUScene::UploadMaterialData(uint32_t FrameIndex)
{
    if (mMaterialData.empty())
    {
        return;
    }

    void* MappedData = mMaterialBuffer[FrameIndex].Map();
    if (MappedData)
    {
        size_t BytesToCopy = mMaterialData.size() * sizeof(FGPUMaterialData);
        memcpy(MappedData, mMaterialData.data(), BytesToCopy);
        mMaterialBuffer[FrameIndex].Unmap();
    }
}

void FGPUScene::UploadPrimitiveData(uint32_t FrameIndex)
{
    if (mPrimitiveData.empty())
    {
        return;
    }

    void* MappedData = mPrimitiveBuffer[FrameIndex].Map();
    if (MappedData)
    {
        size_t BytesToCopy = mPrimitiveData.size() * sizeof(FGPUPrimitiveData);
        memcpy(MappedData, mPrimitiveData.data(), BytesToCopy);
        mPrimitiveBuffer[FrameIndex].Unmap();
    }
}
