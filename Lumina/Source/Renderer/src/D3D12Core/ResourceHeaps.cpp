#include <cassert>
#include "Renderer/D3D12Core/Common.h"
#include "Renderer/D3D12Core/ResourceView.h"
#include "../../include/Renderer/D3D12Core/ResourceHeaps.h"

void StaticResourceViewHeap::Create(ID3D12Device* pDevice, const wchar_t* ResourceName, EResourceHeapType HeapType,
                                    uint32_t Capacity, bool bCPUVisible)
{
    static constexpr D3D12_DESCRIPTOR_HEAP_TYPE HeapTypeTable[] = {
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
    };
    D3D12_DESCRIPTOR_HEAP_TYPE D3D12HeapType = HeapTypeTable[HeapType];

    this->mCapacity = Capacity;
    this->mIsDescriptorFree.resize(Capacity, true);
    this->mDescriptorElementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12HeapType);
    this->mHeapType = HeapType;

    D3D12_DESCRIPTOR_HEAP_DESC DescritporHeapDesc = {};
    DescritporHeapDesc.NumDescriptors = Capacity;
    DescritporHeapDesc.Type = D3D12HeapType;
    DescritporHeapDesc.Flags = ((D3D12HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV) || (D3D12HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV))
        ? D3D12_DESCRIPTOR_HEAP_FLAG_NONE : D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    this->mbGPUVisible = DescritporHeapDesc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (bCPUVisible)
    {
        DescritporHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    }
    DescritporHeapDesc.NodeMask = 0;

    HRESULT HResult = pDevice->CreateDescriptorHeap(&DescritporHeapDesc, IID_PPV_ARGS(&mpHeap));
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "CreateDescriptorHeap() failed");
        return;
    }
    this->mpHeap->SetName(ResourceName);
}

void StaticResourceViewHeap::Destroy()
{
    if (mpHeap)
    {
        mpHeap->Release();
        mpHeap = nullptr;
    }
    mIsDescriptorFree.clear();
}

bool StaticResourceViewHeap::AllocateDescriptor(uint32_t Count, ResourceView* pResourceView)
{
    uint32_t iStart = 0;
    bool bFoundSpace = false;

    while (iStart <= mCapacity - Count)
    {
        bFoundSpace = true;
        for (uint32_t i = 0; i < Count; i++)
        {
            if (!mIsDescriptorFree[iStart + i])
            {
                iStart += i + 1;
                bFoundSpace = false;
                break;
            }
        }

        if (bFoundSpace)
        {
            break;
        }
    }

    if (!bFoundSpace)
    {
        LUMINA_LOG_ERROR(RHI, "CreateDescriptorHeap() failed: couldn't find contiguous descriptor block");
        return false;
    }

    for (uint32_t i = 0; i < Count; i++)
    {
        assert(mIsDescriptorFree[iStart + i]);
        if (!mIsDescriptorFree[iStart + i])
        {
            LUMINA_LOG_WARNING(RHI, "OVERWRITING DESCRIPTOR %d (base=%d+offset=%d)", iStart + i, iStart, i);
        }
        mIsDescriptorFree[iStart + i] = false;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = mpHeap->GetCPUDescriptorHandleForHeapStart();
    CpuHandle.ptr += static_cast<size_t>(iStart) * mDescriptorElementSize;

    D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle = mbGPUVisible ? mpHeap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{};
    GpuHandle.ptr += mbGPUVisible ? static_cast<size_t>(iStart) * mDescriptorElementSize : 0;

    pResourceView->SetResourceView(Count, mDescriptorElementSize, CpuHandle, GpuHandle);

    return true;
}

void StaticResourceViewHeap::FreeDescriptor(ResourceView* pResourceView)
{
    assert(pResourceView);
    if (!pResourceView)
    {
        return;
    }

    const size_t iResourceView = (pResourceView->GetCpuDescriptorHandle().ptr - mpHeap->GetCPUDescriptorHandleForHeapStart().ptr) / mDescriptorElementSize;
    for (uint32_t i = 0; i < pResourceView->GetSize(); i++)
    {
        assert(iResourceView + i < mIsDescriptorFree.size());
        mIsDescriptorFree[iResourceView + i] = true;
    }
}

void UploadHeap::Create(ID3D12Device* pDevice, size_t uSize, Microsoft::WRL::ComPtr<ID3D12CommandQueue> pQueue)
{
    mpDevice = pDevice;
    mpCommandQueue = pQueue;

    // Create CommandList and Command Allocators
    pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mpCommandAllocator));
    mpCommandAllocator->SetName(L"UpLoadHeap::mpCommandAllocator");
    pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mpCommandAllocator, nullptr, IID_PPV_ARGS(&mpCommandList));
    mpCommandList->SetName(L"UploadHeap::mpCommandList");

    // Create Buffer
    D3D12_HEAP_PROPERTIES HeapProperties = {};
    HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProperties.CreationNodeMask = 1;
    HeapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC BufferDesc = {};
    BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    BufferDesc.Alignment = 0;
    BufferDesc.Width = uSize;
    BufferDesc.Height = 1;
    BufferDesc.DepthOrArraySize = 1;
    BufferDesc.MipLevels = 1;
    BufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    BufferDesc.SampleDesc.Count = 1;
    BufferDesc.SampleDesc.Quality = 0;
    BufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    BufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT HResult = {};
    HResult = pDevice->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &BufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&mpUploadHeap)
        );
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "Couldn't create upload heap.");
        return;
    }

    HResult = mpUploadHeap->Map(0, nullptr, (void**)&mpDataBegin);
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "Couldn't map upload heap.");
        return;
    }

    mpDataCurrent = mpDataBegin;
    mpDataEnd = mpDataBegin + mpUploadHeap->GetDesc().Width;

    HResult = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFence));
    mHEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    mFenceValue = 1;
}

void UploadHeap::Destroy()
{
    mpUploadHeap->Release();

    if (mpFence)
    {
        mpFence->Release();
    }

    mpCommandList->Release();
    mpCommandAllocator->Release();
}

uint8_t* UploadHeap::SubAllocate(size_t uSize, uint64_t uAlign)
{
    mpDataCurrent = reinterpret_cast<UINT8*>(AlignOffset(reinterpret_cast<size_t>(mpDataCurrent), (size_t)(uAlign)));

    if (mpDataCurrent >= mpDataEnd || mpDataCurrent + uSize >= mpDataEnd)
    {
        return nullptr;
    }

    uint8_t* pRet = mpDataCurrent;
    mpDataCurrent += uSize;
    return pRet;
}

void UploadHeap::UploadToGPUAndWait(Microsoft::WRL::ComPtr<ID3D12CommandQueue> pQueue)
{
    if (!pQueue)
    {
        pQueue = this->mpCommandQueue;
    }

    mpCommandList->Close();
    ID3D12CommandList* ppCommandLists[] = { mpCommandList };
    pQueue->ExecuteCommandLists(1, ppCommandLists);
    pQueue->Signal(mpFence, mFenceValue);
    if (mpFence->GetCompletedValue() < mFenceValue)
    {
        mpFence->SetEventOnCompletion(mFenceValue, mHEvent);
        WaitForSingleObject(mHEvent, INFINITE);
    }

    ++mFenceValue;

    mpCommandAllocator->Reset();
    mpCommandList->Reset(mpCommandAllocator, nullptr);

    mpDataCurrent = mpDataBegin;
}

