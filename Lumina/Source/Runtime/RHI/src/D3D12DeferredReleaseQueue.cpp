#include "../include/D3D12DeferredReleaseQueue.h"
#include "../include/D3D12CommandQueue.h"

FD3D12CommandQueue* FD3D12DeferredReleaseQueue::mpGraphicsQueue = nullptr;
std::mutex FD3D12DeferredReleaseQueue::mMutex;
std::vector<FD3D12DeferredReleaseQueue::Entry> FD3D12DeferredReleaseQueue::mEntries;
bool FD3D12DeferredReleaseQueue::mbImmediateMode = true;

void FD3D12DeferredReleaseQueue::Initialize(FD3D12CommandQueue *pGraphicsQueue)
{
    std::lock_guard<std::mutex> Lock(mMutex);
    mpGraphicsQueue = pGraphicsQueue;
    mbImmediateMode = (pGraphicsQueue == nullptr);
}

void FD3D12DeferredReleaseQueue::Shutdown()
{
    FlushAll();

    std::lock_guard<std::mutex> Lock(mMutex);
    mpGraphicsQueue = nullptr;
    mbImmediateMode = true;
}

void FD3D12DeferredReleaseQueue::Enqueue(std::function<void()> ReleaseFunc)
{
    if (!ReleaseFunc) return;

    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (!mbImmediateMode && mpGraphicsQueue)
        {
            mEntries.push_back({ mpGraphicsQueue->GetNextFenceValue(), std::move(ReleaseFunc) });
            return;
        }
    }

    ReleaseFunc();
}

void FD3D12DeferredReleaseQueue::Flush()
{
    std::vector<std::function<void()>> Ready;

    {
        std::lock_guard<std::mutex> Lock(mMutex);
        if (!mpGraphicsQueue) return;

        for (size_t i = 0; i < mEntries.size(); )
        {
            if (mpGraphicsQueue->IsFenceComplete(mEntries[i].FenceValue))
            {
                Ready.push_back(std::move(mEntries[i].ReleaseFunc));
                mEntries[i] = std::move(mEntries.back());
                mEntries.pop_back();
            }
            else
            {
                ++i;
            }
        }
    }

    for (auto& Func : Ready) Func();
}

void FD3D12DeferredReleaseQueue::FlushAll()
{
    std::vector<std::function<void()>> All;

    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mpGraphicsQueue)
        {
            mpGraphicsQueue->Flush();
        }

        All.reserve(mEntries.size());
        for (auto& Entry : mEntries) All.push_back(std::move(Entry.ReleaseFunc));
        mEntries.clear();
    }

    for (auto& Func : All) Func();
}

size_t FD3D12DeferredReleaseQueue::GetPendingCount()
{
    std::lock_guard<std::mutex> Lock(mMutex);
    return mEntries.size();
}
