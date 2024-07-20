#pragma once
#include <timeapi.h>
#pragma comment (lib, "Winmm.lib")
UISTONE_BEGIN

class FCPerformanceTest
{
private:
    DWORD   m_start;

public:
    FCPerformanceTest()
    {
        timeBeginPeriod(1);
        Restart();
    }

    ~FCPerformanceTest()
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

    void DebugOut(PCWSTR prefix_txt = L"oxo -- elapse -- ")
    {
        CString   s;
        s.Format(L"%s%d\n", prefix_txt, GetElapseTime());
        ::OutputDebugString(s);
    }
};

UISTONE_END
