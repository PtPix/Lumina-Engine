#include "FrameTimer/FrameTimer.h"

FrameTimer::FrameTimer()
{
    Reset();
}

void FrameTimer::Reset()
{
    mStartTime = std::chrono::high_resolution_clock::now();
    mLastFrameTime = mStartTime;
    mLastReportTime = mStartTime;
    mDeltaTime = 0.0;
    mTotalTime = 0.0;
    mFrameCount = 0;
    mFPS = 0.0;
    mAvgFrameTimeMs = 0.0;
}

void FrameTimer::Tick()
{
    auto CurrentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> DeltaTimeDuration = CurrentTime - mLastFrameTime;
    mDeltaTime = DeltaTimeDuration.count();
    mLastFrameTime = CurrentTime;

    if (mDeltaTime > 0.2)
    {
        mDeltaTime = 0.2;
    }

    mFrameCount++;
}

bool FrameTimer::UpdateAndCheckReportInterval()
{
    auto CurrentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> TimeSinceReport = CurrentTime - mLastReportTime;

    if (TimeSinceReport.count() >= 1.0)
    {
        mFPS = mFrameCount / TimeSinceReport.count();
        mAvgFrameTimeMs = 1000.0 / mFPS;
        mTotalTime = std::chrono::duration<double>(CurrentTime - mStartTime).count();

        mFrameCount = 0;
        mLastReportTime = CurrentTime;
        return true;
    }

    return false;
}