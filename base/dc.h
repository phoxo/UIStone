#pragma once
#include <optional>

class AutoRestoreHDC
{
private:
    HDC   m_dc;

public:
    AutoRestoreHDC(HDC dc, std::optional<int> select_stock_obj = std::nullopt) : m_dc(dc)
    {
        ::SaveDC(dc);
        if (select_stock_obj)
        {
            SelectObject(dc, GetStockObject(*select_stock_obj));
        }
    }

    ~AutoRestoreHDC()
    {
        ::RestoreDC(m_dc, -1);
    }
};

#ifdef _AFX
class BufferedPaintDC : public CDC
{
private:
    HPAINTBUFFER   m_paint;

public:
    BufferedPaintDC(HDC dc, LPCRECT rc)
    {
        HDC   tmpdc = NULL;
        m_paint = BeginBufferedPaint(dc, rc, BPBF_COMPATIBLEBITMAP, NULL, &tmpdc);
        Attach(tmpdc);
    }

    ~BufferedPaintDC()
    {
        Detach();
        EndBufferedPaint(m_paint, TRUE);
    }
};
#endif
