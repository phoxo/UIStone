#pragma once

/// Composite two straight alpha images (32 bit).
class FCEffectCompositeStraight : public FCImageEffect
{
private:
    const FCImage   & m_top;
    POINT   m_top_position;
    int   m_percent;

public:
    FCEffectCompositeStraight(const FCImage& top, POINT top_position, int top_percent = 100) : m_top(top)
    {
        m_top_position = top_position;
        m_percent = std::clamp(top_percent, 0, 100);
    }

private:
    virtual bool IsSupport(const FCImage& img)
    {
        return (img.ColorBits() == 32) && (m_top.ColorBits() == 32) && !img.IsPremultiplied() && !m_top.IsPremultiplied();
    }

    virtual ProcessMode QueryProcessMode()
    {
        return ProcessMode::Region;
    }

    virtual void ProcessRegion(FCImage& img, CRect region_rect, FCProgressObserver* progress)
    {
        CRect   rect_top(m_top_position, CSize(m_top.Width(), m_top.Height()));
        CRect   rc;
        if (!rc.IntersectRect(region_rect, rect_top))
            return;

        for (int y = rc.top; y < rc.bottom; y++)
        {
            auto   dest = (RGBA32bit*)img.GetBits(rc.left, y);
            auto   src = (RGBA32bit*)m_top.GetBits(rc.left - m_top_position.x, y - m_top_position.y);
            for (int x = rc.left; x < rc.right; x++, dest++, src++)
            {
                RGBA32bit   cr = *src;
                if (m_percent != 100)
                {
                    cr.a = (BYTE)(cr.a * m_percent / 100);
                }
                FCColor::CompositeStraightAlpha(*dest,  cr);
            }
        }
    }
};
