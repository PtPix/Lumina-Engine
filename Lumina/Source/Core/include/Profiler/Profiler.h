#pragma once
#include <chrono>

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
