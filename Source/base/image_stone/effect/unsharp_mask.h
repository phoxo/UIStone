#pragma once

/// UNSharp mask (32 bit).
class FCEffectUNSharpMask : public FCImageEffect
{
private:
    FCImage   m_bak;
    int   m_amount;
    int   m_threshold;
    FCEffectGaussianBlur   m_first_blur;

public:
    /// radius >= 1 \n 1 <= amount <= 100 \n 0 <= threshold <= 255.
    FCEffectUNSharpMask(int radius, int amount, int threshold) : m_first_blur(radius, true)
    {
        m_amount = std::clamp(amount, 1, 100);
        m_threshold = std::clamp(threshold, 0, 0xFF);
    }

private:
    virtual bool IsSupport(const FCImage& img)
    {
        return (img.ColorBits() == 32) && !img.IsPremultiplied();
    }

    virtual ProcessMode QueryProcessMode()
    {
        return ProcessMode::PixelByPixel;
    }

    virtual void OnBeforeProcess(FCImage& img)
    {
        m_bak = img;

        m_first_blur.EnableParallelAccelerate(IsParallelAccelerateEnable());
        img.ApplyEffect(m_first_blur);
    }

    virtual void ProcessPixel(FCImage& img, int x, int y, RGBA32bit* pixel)
    {
        auto   oldpx = (BYTE*)m_bak.GetBits(x, y);
        auto   newpx = (BYTE*)pixel;
        for (int i = 0; i < 3; i++)
        {
            int   d = oldpx[i] - newpx[i];
            if (abs(2 * d) < m_threshold)
                d = 0;
            newpx[i] = FCMath::Clamp0255(oldpx[i] + m_amount * d / 100);
        }
        pixel->a = oldpx[3];
    }
};
