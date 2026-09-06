/**
  * @file GPUScene.h
  * @brief GPU Scene manager for unified scene data on GPU.
  *
  * Manages all scene data (instances, materials, primitives) on GPU.
  * Supports dynamic updates and Bindless access.
  */

#pragma once

#include "GPUSceneData.h"
#include "D3D12Buffer.h"
#include "RenderTypes.h"
#include <vector>
#include <unordered_map>

class FD3D12Device;
namespace D3D12MA { class Allocator; }

struct FGPUSceneConfig
{
    uint32_t MaxInstances = 10000;
    uint32_t MaxMaterials = 1000;
    uint32_t MaxPrimitives = 10000;
};

/**
   * @brief GPU Scene manager.
   *
   * Centralizes all scene data on GPU:
   * - Instance data (transforms, material indices)
   * - Material data (PBR parameters, texture indices)
   * - Primitive data (bounds, flags)
   *
   * All data is uploaded as StructuredBuffers and accessed via Bindless indices.
   */
class FGPUScene
{
public:
    FGPUScene() = default;
    ~FGPUScene() { Shutdown(); }

    FGPUScene(const FGPUScene&) = delete;
    FGPUScene& operator=(const FGPUScene&) = delete;

    bool Initialize(FD3D12Device* pDevice, const FGPUSceneConfig& Config = {});
    void Shutdown();

    [[nodiscard]] bool IsInitialized() const { return mbInitialized; }

    // ------------------------------------------------------------------------
    // Primitive Management
    // ------------------------------------------------------------------------
    /**
       * @brief Add a new primitive to the scene.
       * @return Primitive ID (unique identifier).
       */
    uint32_t AddPrimitive(const FGPUInstanceData& InstanceData, const FGPUPrimitiveData& PrimitiveData);

    /**
       * @brief Update an existing primitive.
       */
    void UpdatePrimitive(uint32_t PrimitiveID, const FGPUInstanceData& InstanceData);

    /**
       * @brief Remove a primitive from the scene.
       */
    void RemovePrimitive(uint32_t PrimitiveID);

    // ------------------------------------------------------------------------
    // Material Management
    // ------------------------------------------------------------------------
    /**
       * @brief Add a new material.
       * @return Material index (auto-assigned).
       */
    uint32_t AddMaterial(const FGPUMaterialData& MaterialData);

    /**
       * @brief Update an existing material.
       * @param MaterialIndex The material index to update.
       */
    void UpdateMaterial(uint32_t MaterialIndex, const FGPUMaterialData& MaterialData);

    // ------------------------------------------------------------------------
    // Upload to GPU
    // ------------------------------------------------------------------------
    /**
      * @brief Upload pending data to GPU.
      * Call this once per frame before rendering.
      */
    void UploadToGPU(uint32_t FrameIndex);

    // ------------------------------------------------------------------------
    // Misc
    // ------------------------------------------------------------------------
    [[nodiscard]] uint32_t GetInstanceBufferSRV() const { return mInstanceBufferSRVIndex; }
    [[nodiscard]] uint32_t GetMaterialBufferSRV() const { return mMaterialBufferSRVIndex; }
    [[nodiscard]] uint32_t GetPrimitiveBufferSRV() const { return mPrimitiveBufferSRVIndex; }

    [[nodiscard]] uint32_t GetInstanceCount() const { return static_cast<uint32_t>(mInstanceData.size()); }
    [[nodiscard]] uint32_t GetMaterialCount() const { return static_cast<uint32_t>(mMaterialData.size()); }
    [[nodiscard]] uint32_t GetPrimitiveCount() const { return static_cast<uint32_t>(mPrimitiveData.size()); }

private:
    bool CreateBuffers();
    void UploadInstanceData(uint32_t FrameIndex);
    void UploadMaterialData(uint32_t FrameIndex);
    void UploadPrimitiveData(uint32_t FrameIndex);

private:
    FD3D12Device* mpDevice = nullptr;
    D3D12MA::Allocator* mpAllocator = nullptr;

    FGPUSceneConfig mConfig = {};

    // CPU-side data (pending upload)
    std::vector<FGPUInstanceData> mInstanceData;
    std::vector<FGPUMaterialData> mMaterialData;
    std::vector<FGPUPrimitiveData> mPrimitiveData;

    // Map: PrimitiveID -> Index;
    std::unordered_map<uint32_t, uint32_t> mPrimitiveIDToIndex;
    uint32_t mNextPrimitiveID = 1;

   uint32_t mNextMaterialIndex = 0;

    // GPU Buffers (per-frame for dynamic updates)
    FD3D12StructuredBuffer mInstanceBuffer[NUM_FRAMES_IN_FLIGHT];
    FD3D12StructuredBuffer mMaterialBuffer[NUM_FRAMES_IN_FLIGHT];
    FD3D12StructuredBuffer mPrimitiveBuffer[NUM_FRAMES_IN_FLIGHT];

    // Bindless SRV indices
    uint32_t mInstanceBufferSRVIndex = FD3D12StructuredBuffer::InvalidBindlessIndex;
    uint32_t mMaterialBufferSRVIndex = FD3D12StructuredBuffer::InvalidBindlessIndex;
    uint32_t mPrimitiveBufferSRVIndex = FD3D12StructuredBuffer::InvalidBindlessIndex;

    // Dirty Flags
    bool mbInstanceDataDirty = false;
    bool mbMaterialDataDirty = false;
    bool mbPrimitiveDataDirty = false;

    bool mbInitialized = false;
};
