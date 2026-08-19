#pragma once

#include  <cstdint>
#include <functional>
#include <mutex>
#include <vector>

class FCommandQueue;

class FDeferredReleaseQueue
{
public:
    static void Initialize(FCommandQueue* pGraphicsQueue);
    static void Shutdown();

    static void Enqueue(std::function<void()> ReleaseFunc);

    static void Flush();
    static void FlushAll();

    [[nodiscard]] static size_t GetPendingCount();

private:
    struct Entry
    {
        uint64_t FenceValue = 0;
        std::function<void()> ReleaseFunc = nullptr;
    };

    static FCommandQueue* mpGraphicsQueue;
    static std::mutex mMutex;
    static std::vector<Entry> mEntries;
    static bool mbImmediateMode;
};