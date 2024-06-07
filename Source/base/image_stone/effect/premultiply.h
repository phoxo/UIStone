#pragma once

/// Premultiply image (32 bit).
class FCEffectPremultiply : public FCImageEffect
{
private:
    virtual bool IsSupport(const FCImage& img)
    {
        return (img.ColorBits() == 32) && !img.IsPremultiplied();
    }

    virtual ProcessMode QueryProcessMode()
    {
        return ProcessMode::Region;
    }

    virtual void ProcessRegion(FCImage& img, CRect rc, FCProgressObserver* progress)
    {
        for (int y = rc.top; y < rc.bottom; y++)
        {
            auto   px = (RGBA32bit*)img.GetBits(rc.left, y);
            for (int x = rc.left; x < rc.right; x++, px++)
            {
                FCColor::Premultiply(*px);
            }
        }
    }

    virtual void OnAfterProcess(FCImage& img)
    {
        img.SetPremultiplied(true);
    }
};

/// UnPremultiply image (32 bit).
class FCEffectUnPremultiply : public FCImageEffect
{
private:
    virtual bool IsSupport(const FCImage& img)
    {
        return (img.ColorBits() == 32) && img.IsPremultiplied();
    }

    virtual ProcessMode QueryProcessMode()
    {
        return ProcessMode::Region;
    }

    virtual void ProcessRegion(FCImage& img, CRect rc, FCProgressObserver* progress)
    {
        for (int y = rc.top; y < rc.bottom; y++)
        {
            auto   px = (RGBA32bit*)img.GetBits(rc.left, y);
            for (int x = rc.left; x < rc.right; x++, px++)
            {
                FCColor::UnPremultiply(*px);
            }
        }
    }

    virtual void OnAfterProcess(FCImage& img)
    {
        img.SetPremultiplied(false);
    }
};
