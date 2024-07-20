#pragma once
UISTONE_BEGIN

class CDPICalculator
{
private:
    static int AcquireDpi()
    {
        HDC   dc = ::GetDC(NULL);
        int   dpi = GetDeviceCaps(dc, LOGPIXELSX);
        ::ReleaseDC(NULL, dc);
        return dpi;
    }

public:
    static int& g_current_dpi()
    {
        static int   s = AcquireDpi();
        return s;
    }

    static float GetDPIScaleFactor()
    {
        return g_current_dpi() / (float)USER_DEFAULT_SCREEN_DPI;
    }

    static int Cast(int v, int v_designed_for_dpi = USER_DEFAULT_SCREEN_DPI)
    {
        if (!v_designed_for_dpi)
        {
            assert(false);
            return 1;
        }

        return v * g_current_dpi() / v_designed_for_dpi;
    }
};

UISTONE_END
