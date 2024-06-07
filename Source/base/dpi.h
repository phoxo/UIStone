#pragma once
_UISTONE_BEGIN

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

    static int Cast(int v, int v_designed_for_dpi = USER_DEFAULT_SCREEN_DPI)
    {
        if (!v_designed_for_dpi)
        {
            ASSERT(FALSE);
            return 1;
        }

        return v * g_current_dpi() / v_designed_for_dpi;
    }
};

_UISTONE_END
