#include "Renderer/D3D12Core/Core/DeferredReleaseQueue.h"
#include "Renderer/D3D12Core/Core/CommandQueue.h"

FCommandQueue* FDeferredReleaseQueue::mpGraphicsQueue = nullptr;
std::mutex FDeferredReleaseQueue::mMutex;
std::vector<FDeferredReleaseQueue::Entry> FDeferredReleaseQueue::mEntries;
bool FDeferredReleaseQueue::mbImmediateMode = true;

void FDeferredReleaseQueue::Initialize(FCommandQueue *pGraphicsQueue)
{
    std::lock_guard<std::mutex> Lock(mMutex);
    mpGraphicsQueue = pGraphicsQueue;
    mbImmediateMode = (pGraphicsQueue == nullptr);
}

void FDeferredReleaseQueue::Shutdown()
{
    FlushAll();

    std::lock_guard<std::mutex> Lock(mMutex);
    mpGraphicsQueue = nullptr;
    mbImmediateMode = true;
}

void FDeferredReleaseQueue::Enqueue(std::function<void()> ReleaseFunc)
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

void FDeferredReleaseQueue::Flush()
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

void FDeferredReleaseQueue::FlushAll()
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

size_t FDeferredReleaseQueue::GetPendingCount()
{
    std::lock_guard<std::mutex> Lock(mMutex);
    return mEntries.size();
}
