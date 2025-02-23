#pragma once

/// Font helper.
class FontManager
{
private:
    class CAutoFont
    {
    public:
        HFONT   m_font = NULL;
        CAutoFont()
        {
            NONCLIENTMETRICS   nm = { sizeof(NONCLIENTMETRICS) };
            SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0);
#ifdef _AFX
            CFont   tmp;
            tmp.CreatePointFont(90, nm.lfMenuFont.lfFaceName);
            m_font = (HFONT)tmp.Detach();
#endif
        }
        ~CAutoFont()
        {
            ::DeleteObject(m_font);
            m_font = NULL;
        }
    };

public:
    /// Get default UI font (font used by menu), <B>mustn't delete returned font</B>.
    static HFONT GetDefaultFont()
    {
        return GLOBAL_OBJ().m_font;
    }

    /// Get face name of default font.
    static CString GetDefaultFontFaceName()
    {
        LOGFONT   lf = {};
        ::GetObject(GetDefaultFont(), sizeof(lf), &lf);
        return lf.lfFaceName;
    }

    static CSize MeasureStringSize(PCWSTR str, HFONT str_font)
    {
#ifdef _AFX
        CDC   dc;
        dc.CreateCompatibleDC(NULL);
        auto   old_font = SelectObject(dc, str_font);
        CSize   sz = dc.GetTextExtent(str);
        SelectObject(dc, old_font);
        return sz;
#else
        return CSize();
#endif
    }

private:
    static CAutoFont& GLOBAL_OBJ()
    {
        static CAutoFont   v;
        return v;
    }
};
