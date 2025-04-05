#pragma once
#include <timeapi.h>
#pragma comment (lib, "Winmm.lib")

class PerformanceTest
{
private:
    DWORD   m_start;

public:
    PerformanceTest()
    {
        timeBeginPeriod(1);
        Restart();
    }

    ~PerformanceTest()
    {
        timeEndPeriod(1);
    }

    void Restart()
    {
        m_start = timeGetTime();
    }

    int GetElapseTime() const
    {
        INT64   n = timeGetTime() - (INT64)m_start;
        if (n < 0)
            n += 0xFFFFFFFFi64;
        return (int)n;
    }

    void DebugOut(PCWSTR prefix_txt = L"oxo -- ")
    {
        CString   s;
        s.Format(L"%s%d\n", prefix_txt, GetElapseTime());
        ::OutputDebugString(s);
    }
};
