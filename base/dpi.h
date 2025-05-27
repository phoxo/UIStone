#pragma once

class DPICalculator
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
    static const int& g_current_dpi()
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
            return v;
        }

        return v * g_current_dpi() / v_designed_for_dpi;
    }

    static int Cast(HWND wnd, int v, int v_designed_for_dpi = USER_DEFAULT_SCREEN_DPI)
    {
        if (UINT dpi = GetDpiForWindow(wnd))
        {
            return v * dpi / v_designed_for_dpi;
        }
        return v;
    }
};
