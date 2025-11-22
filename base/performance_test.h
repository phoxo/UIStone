#pragma once
#include <chrono>

class PerformanceTest
{
private:
    struct AverageCount
    {
        double   total = 0;
        double   count = 0;

        int Update(int curr)
        {
            total += curr;
            count++;
            return (int)(total / count);
        }
    };

private:
    using stlclock = std::chrono::steady_clock;

    stlclock::time_point   m_start;
    std::unique_ptr<AverageCount>   m_average;

public:
    PerformanceTest()
    {
        Restart();
    }

    void SetAverageMode()
    {
        m_average = std::make_unique<AverageCount>();
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
        int   curr = GetElapseTime();
        if (m_average)
        {
            curr = m_average->Update(curr);
        }

        CString   s;
        s.Format(L"%s%d\n", prefix_txt, curr);
        ::OutputDebugString(s);
    }
};
