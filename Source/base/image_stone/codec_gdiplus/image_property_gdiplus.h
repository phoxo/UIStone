#pragma once

struct CImagePropertyGdiplus
{
public:
    int   m_save_jpeg_quality = 0; // 1 <= n <= 100
    int   m_dpi = 0;

public:
    void AddAllProperty(Gdiplus::Bitmap& bmp) const
    {
        if (m_dpi)
        {
            bmp.SetResolution((float)m_dpi, (float)m_dpi);
        }
    }
};
