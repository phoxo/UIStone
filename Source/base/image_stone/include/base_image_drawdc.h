#pragma once

/// Memory DC with auto bitmap select.
class FCImageDrawDC
{
private:
    HDC      m_dc;
    HGDIOBJ  m_old;
    HGDIOBJ  m_font_bak;

public:
    /// create memory dc and select bmp in.
    FCImageDrawDC(HBITMAP bmp, HBRUSH fill_background = NULL)
    {
        m_dc = CreateCompatibleDC(NULL);
        m_old = SelectObject(m_dc, bmp);
        m_font_bak = GetCurrentObject(m_dc, OBJ_FONT);
        SetBkMode(m_dc, TRANSPARENT);
        SetStretchBltMode(m_dc, COLORONCOLOR);
        if (fill_background)
        {
            ::FillRect(m_dc, CRect(CPoint(), GetBitmapSize(bmp)), fill_background);
        }
    }

    ~FCImageDrawDC()
    {
        SelectObject(m_dc, m_font_bak);
        SelectObject(m_dc, m_old);
        DeleteDC(m_dc);
    }

    /// get HDC handle.
    operator HDC() const { return m_dc; }

    static CSize GetBitmapSize(HBITMAP bmp)
    {
        BITMAP   bm = {};
        ::GetObject(bmp, sizeof(bm), &bm);
        return CSize(bm.bmWidth, bm.bmHeight);
    }
};
