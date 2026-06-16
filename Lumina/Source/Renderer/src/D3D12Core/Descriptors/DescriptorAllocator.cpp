#include "Renderer/D3D12Core/Descriptors/DescriptorAllocator.h"
#include "Renderer/D3D12Core/Descriptors/DescriptorAllocatorPage.h"

FDescriptorAllocator::FDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT DescriptorsPerPage)
    : mHeapType(Type), mDescriptorsPerPage(DescriptorsPerPage)
{
}

void FDescriptorAllocator::Initialize(FDevice* pDevice)
{
    mpDevice = pDevice;
}

FDescriptorAllocation FDescriptorAllocator::Allocate(UINT NumDescriptors)
{
    std::lock_guard<std::mutex> Lock(mAllocationMutex);

    for (const auto& Page : mHeapPool)
    {
        if (Page->HasSpace(NumDescriptors))
        {
            FDescriptorAllocation Allocation = Page->Allocate(NumDescriptors);
            if (Allocation.IsValid())
            {
                return Allocation;
            }
        }
    }

    auto NewPage = CreateAllocatorPage();
    return NewPage->Allocate(NumDescriptors);
}

std::shared_ptr<FDescriptorAllocatorPage> FDescriptorAllocator::CreateAllocatorPage()
{
    auto NewPage = std::make_shared<FDescriptorAllocatorPage>(mpDevice, mHeapType, mDescriptorsPerPage);
    mHeapPool.push_back(NewPage);

    return NewPage;
}
