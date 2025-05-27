#pragma once
#include <chrono>

class PerformanceTest
{
private:
    using stlclock = std::chrono::steady_clock;

    stlclock::time_point   m_start;

public:
    PerformanceTest()
    {
        Restart();
    }

    void Restart()
    {
        m_start = stlclock::now();
    }

    int GetElapseTime() const
    {
        auto   elapsed = stlclock::now() - m_start;
        return (int)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    }

    void DebugOut(PCWSTR prefix_txt = L"oxo -- ")
    {
        CString   s;
        s.Format(L"%s%d\n", prefix_txt, GetElapseTime());
        ::OutputDebugString(s);
    }
};
