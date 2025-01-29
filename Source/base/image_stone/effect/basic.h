#pragma once

/// Grayscale (24 or 32 bit).
class FCEffectGrayscale : public FCImageEffect
{
    virtual void ProcessPixel(FCImage& img, int x, int y, RGBA32bit* px)
    {
        px->r = px->g = px->b = FCColor::GetGrayscale(px);
    }
};

/// Fill color (24 or 32 bit).
class FCEffectFillColor : public FCImageEffect
{
private:
    RGBA32bit   m_color;
public:
    /// if 32bpp, the alpha channel will be filled.
    FCEffectFillColor(FCColor color) : m_color{ color } {}
private:
    virtual void ProcessPixel(FCImage& img, int x, int y, RGBA32bit* px)
    {
        if (img.ColorBits() == 32)
            *px = m_color;
        else
            *(RGBTRIPLE*)px = *(RGBTRIPLE*)&m_color;
    }
};

namespace imagestone_inner
{
    /// LUT(look up table) routine (24 or 32 bit).
    class FCEffectLUT : public FCImageEffect
    {
    private:
        BYTE   m_LUT[256] = {};
    protected:
        /// 0 <= idx <= 0xFF
        virtual BYTE InitLUTtable(int idx) = 0;
    public:
        virtual void OnBeforeProcess(FCImage& img)
        {
            for (int i = 0; i <= 0xFF; i++)
            {
                m_LUT[i] = InitLUTtable(i);
            }
        }
        virtual void ProcessPixel(FCImage& img, int x, int y, RGBA32bit* px)
        {
            px->b = m_LUT[px->b];
            px->g = m_LUT[px->g];
            px->r = m_LUT[px->r];
        }
    };
}

/// Brightness and contrast (24 or 32 bit).
class FCEffectBrightnessContrast : public imagestone_inner::FCEffectLUT
{
private:
    int   m_brightness;
    int   m_contrast;
public:
    /// -100 <= brightness <= 100, 0 means not change\n -100 <= contrast <= 100, 0 means not change.
    FCEffectBrightnessContrast(int brightness, int contrast)
    {
        m_brightness = std::clamp(brightness, -100, 100);
        m_contrast = std::clamp(contrast, -100, 100);
    }
private:
    virtual BYTE InitLUTtable(int idx)
    {
        double   d = (100 + m_contrast) / 100.0;
        int   n = (int)((idx - 128) * d + (m_brightness + 128) + 0.5);
        return FCMath::Clamp0255(n);
    }
};

/// Invert color (24 or 32 bit).
class FCEffectInvertColor : public imagestone_inner::FCEffectLUT
{
    virtual BYTE InitLUTtable(int idx)
    {
        return (BYTE)(255 - idx);
    }
};
