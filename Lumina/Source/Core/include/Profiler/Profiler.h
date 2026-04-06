#pragma once
#include <chrono>

// Frame Timer, Output FPS
class FrameTimer
{
public:
    FrameTimer();

    void Reset();
    void Tick();
    bool UpdateAndCheckReportInterval();

    double GetDeltaTime() const { return mDeltaTime; }
    double GetTotalTime() const { return mTotalTime; }
    double GetFPS() const { return mFPS; }
    double GetAvgFrameTimeMs() const { return mAvgFrameTimeMs; }

private:
    std::chrono::high_resolution_clock::time_point mStartTime;
    std::chrono::high_resolution_clock::time_point mLastFrameTime;
    std::chrono::high_resolution_clock::time_point mLastReportTime;

    double mDeltaTime = 0.0;
    double mTotalTime = 0.0;
    uint32_t mFrameCount = 0;
    double mFPS = 0.0;
    double mAvgFrameTimeMs = 0.0;
};

// Time Scope
class FTimeLogScope
{
public:
    FTimeLogScope(const char* InStaticLabel)
        : Label(InStaticLabel)
        , StartTime(std::chrono::high_resolution_clock::now())
    {}

    ~FTimeLogScope();

    FTimeLogScope(const FTimeLogScope&) = delete;
    FTimeLogScope& operator=(const FTimeLogScope&) = delete;

private:
    const char* const Label;
    const std::chrono::high_resolution_clock::time_point StartTime;
};

#define LUMINA_TIME_LOG_CONCAT_IMPL(x, y) x##y
#define LUMINA_TIME_LOG_CONCAT(x, y) LUMINA_TIME_LOG_CONCAT_IMPL(x, y)

#define LUMINA_TIME_LOG_SCOPE(Name) FTimeLogScope LUMINA_TIME_LOG_CONCAT(TimeLog_, __LINE__)(Name)
