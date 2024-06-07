#pragma once

/// Rotate 90 degree clockwise (24 or 32 bit).
class FCEffectRotate90 : public FCImageEffect
{
protected:
    FCImage   m_bak;

protected:
    virtual void ProcessPixel(FCImage& img, int x, int y, RGBA32bit* px)
    {
        auto   src = (RGBA32bit*)m_bak.GetBits(y, img.Width() - 1 - x);
        if (img.ColorBits() == 32)
            *px = *src;
        else
            *(RGBTRIPLE*)px = *(RGBTRIPLE*)src;
    }

private:
    virtual void OnBeforeProcess(FCImage& img)
    {
        img.SwapImage(m_bak);
        img.Create(m_bak.Height(), m_bak.Width(), m_bak.ColorBits(), m_bak.GetAttribute());
    }
};

/// Rotate 270 degree clockwise (24 or 32 bit).
class FCEffectRotate270 : public FCEffectRotate90
{
    virtual void ProcessPixel(FCImage& img, int x, int y, RGBA32bit* px)
    {
        auto   src = (RGBA32bit*)m_bak.GetBits(img.Height() - 1 - y, x);
        if (img.ColorBits() == 32)
            *px = *src;
        else
            *(RGBTRIPLE*)px = *(RGBTRIPLE*)src;
    }
};
