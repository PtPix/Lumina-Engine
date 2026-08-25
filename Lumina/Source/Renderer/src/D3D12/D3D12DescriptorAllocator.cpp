#include "Renderer/D3D12/D3D12DescriptorAllocator.h"
#include "Renderer/D3D12/D3D12DescriptorAllocatorPage.h"

FD3D12DescriptorAllocator::FD3D12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT DescriptorsPerPage)
    : mHeapType(Type), mDescriptorsPerPage(DescriptorsPerPage)
{
}

void FD3D12DescriptorAllocator::Initialize(FD3D12Device* pDevice)
{
    mpDevice = pDevice;
}

FD3D12DescriptorAllocation FD3D12DescriptorAllocator::Allocate(UINT NumDescriptors)
{
    std::lock_guard<std::mutex> Lock(mAllocationMutex);

    for (const auto& Page : mHeapPool)
    {
        if (Page->HasSpace(NumDescriptors))
        {
            FD3D12DescriptorAllocation Allocation = Page->Allocate(NumDescriptors);
            if (Allocation.IsValid())
            {
                return Allocation;
            }
        }
    }

    auto NewPage = CreateAllocatorPage();
    return NewPage->Allocate(NumDescriptors);
}

std::shared_ptr<FD3D12DescriptorAllocatorPage> FD3D12DescriptorAllocator::CreateAllocatorPage()
{
    auto NewPage = std::make_shared<FD3D12DescriptorAllocatorPage>(mpDevice, mHeapType, mDescriptorsPerPage);
    mHeapPool.push_back(NewPage);

    return NewPage;
}
