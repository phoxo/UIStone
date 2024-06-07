#pragma once

/// Resize image using GDI+ (24 or 32 bit).
class FCEffectResizeGdiplus : public FCImageEffect
{
private:
    CSize   m_new_size;

public:
    Gdiplus::InterpolationMode   m_resize_mode = Gdiplus::InterpolationModeHighQualityBicubic;

public:
    FCEffectResizeGdiplus(int width, int height) : m_new_size(width, height) {}
    FCEffectResizeGdiplus(CSize new_size) : m_new_size(new_size) {}

private:
    virtual bool IsSupport(const FCImage& img)
    {
        return (img.ColorBits() >= 24);
    }

    virtual ProcessMode QueryProcessMode()
    {
        return ProcessMode::EntireMyself;
    }

    virtual void ProcessEntire(FCImage& img, FCProgressObserver* progress)
    {
        FCImage   old;
        img.SwapImage(old);
        auto   src = FCCodecGdiplus::CreateBitmapReference(old);

        img.Create(m_new_size.cx, m_new_size.cy, old.ColorBits(), old.GetAttribute());
        auto   dest = FCCodecGdiplus::CreateBitmapReference(img);
        if (src && dest)
        {
            Gdiplus::Graphics   gc(dest.get());
            DrawImage(gc, m_new_size, *src);
        }
    }

    void DrawImage(Gdiplus::Graphics& gc, CSize dest_size, Gdiplus::Bitmap& img)
    {
        Gdiplus::ImageAttributes   attri;
        attri.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
        Gdiplus::Rect   drc{ 0, 0, dest_size.cx, dest_size.cy };
        gc.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
        gc.SetInterpolationMode(m_resize_mode);
        gc.DrawImage(&img, drc, 0, 0, img.GetWidth(), img.GetHeight(), Gdiplus::UnitPixel, &attri, NULL, NULL);
    }
};
