#pragma once

/// Resize image using WIC (32bpp).
class FCEffectResizeWic : public FCImageEffect
{
private:
    CSize   m_new_size;

public:
    WICBitmapInterpolationMode   m_resize_mode = WICBitmapInterpolationModeHighQualityCubic;

public:
    FCEffectResizeWic(int width, int height) : m_new_size(width, height) {}
    FCEffectResizeWic(CSize new_size) : m_new_size(new_size) {}

private:
    virtual bool IsSupport(const FCImage& img)
    {
        return (img.ColorBits() == 32);
    }

    virtual ProcessMode QueryProcessMode()
    {
        return ProcessMode::EntireMyself;
    }

    virtual void ProcessEntire(FCImage& img, FCProgressObserver* progress)
    {
        auto   format = (img.IsPremultiplied() ? WICPremultiplied32bpp : WICNormal32bpp);
        auto   old = CWICFunc::CreateBitmapFromHBITMAP(img, img.IsPremultiplied() ? WICBitmapUsePremultipliedAlpha : WICBitmapUseAlpha);
        img.Destroy();
        auto   scaled = CWICFunc::ScaleBitmap(old, m_new_size, m_resize_mode);
        FCCodecWIC::Load(scaled, img, format);
    }
};
