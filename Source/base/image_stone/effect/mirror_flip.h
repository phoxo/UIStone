#pragma once

/// Left right mirror (24 or 32 bit).
class FCEffectMirror : public FCImageEffect
{
    virtual void ProcessPixel(FCImage& img, int x, int y, RGBA32bit* px)
    {
        if (x < img.Width() / 2)
        {
            auto   rp = (RGBA32bit*)img.GetBits(img.Width() - 1 - x, y);
            if (img.ColorBits() == 32)
                std::swap(*px, *rp);
            else
                std::swap(*(RGBTRIPLE*)px, *(RGBTRIPLE*)rp);
        }
    }
};

/// Top bottom flip (24 or 32 bit).
class FCEffectFlip : public FCImageEffect
{
    virtual void ProcessPixel(FCImage& img, int x, int y, RGBA32bit* px)
    {
        if (y < img.Height() / 2)
        {
            auto   bp = (RGBA32bit*)img.GetBits(x, img.Height() - 1 - y);
            if (img.ColorBits() == 32)
                std::swap(*px, *bp);
            else
                std::swap(*(RGBTRIPLE*)px, *(RGBTRIPLE*)bp);
        }
    }
};
