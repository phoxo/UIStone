#pragma once
/// @cond
#include <shlwapi.h>
#pragma comment (lib, "Shlwapi.lib")
/// @endcond

class FCBaseHelper
{
public:
    static void InitBitmapInfoHeader(BITMAPINFOHEADER& v, int width, int height, int bpp, bool top_to_bottom = true)
    {
        ZeroMemory(&v, sizeof(v));
        v.biSize = sizeof(v);
        v.biWidth = width;
        v.biHeight = (top_to_bottom ? -abs(height) : abs(height));
        v.biPlanes = 1;
        v.biBitCount = (WORD)bpp;
    }

    // typical : mod = (HMODULE)&__ImageBase
    static IStreamPtr LoadResource(UINT nID, PCWSTR resource_type, HMODULE mod = NULL)
    {
        auto   hres = FindResource(mod, MAKEINTRESOURCE(nID), resource_type);
        auto   ptr = LockResource(::LoadResource(mod, hres));
        return CreateMemStream(ptr, SizeofResource(mod, hres));
    }

    static CRect CalculateFitWindow(SIZE obj_size, const CRect& wnd_rect)
    {
        int   w = wnd_rect.Width();
        int   h = wnd_rect.Height();
        if (obj_size.cx && obj_size.cy && (w > 0) && (h > 0))
        {
            if ((obj_size.cx > w) || (obj_size.cy > h))
            {
                double   dx = w / (double)obj_size.cx;
                double   dy = h / (double)obj_size.cy;
                double   d = (std::min)(dx, dy);
                obj_size.cx = (std::max)((int)(obj_size.cx * d), 1);
                obj_size.cy = (std::max)((int)(obj_size.cy * d), 1);
            }

            CRect   rc;
            rc.left = wnd_rect.left + (w - obj_size.cx) / 2;
            rc.top = wnd_rect.top + (h - obj_size.cy) / 2;
            rc.BottomRight() = rc.TopLeft() + obj_size;
            return rc;
        }
        return CRect();
    }

    static ULONG __DEBUG_QueryCurrentRefCount(IUnknown* p)
    {
        p->AddRef();
        return p->Release();
    }

    static bool IsValidDate(int year, int month, int day)
    {
        return (year > 1970) && (year < 2900) && (month >= 1) && (month <= 12) && (day >= 1) && (day <= 31);
    }

    static IStreamPtr CreateMemStream(const void* ptr, UINT mem_size)
    {
        IStreamPtr   v;
        if (ptr) { v.Attach(SHCreateMemStream((const BYTE*)ptr, mem_size)); }
        return v;
    }

    template<class T>
    static T& LazyCreateSingleton(std::unique_ptr<T>& obj)
    {
        if (!obj)
        {
            obj = std::make_unique<T>();
        }
        return *obj;
    }
};
