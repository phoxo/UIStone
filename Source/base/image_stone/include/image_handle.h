#pragma once

/// Helper class
class FCImageHandle
{
public:
    static bool IsAllPixelAlpha0xFF(const FCImage& img)
    {
        if (img.ColorBits() != 32)
            return false;

        int   pixel_count = img.Width() * img.Height();
        auto   ptr = (RGBA32bit*)img.GetMemStart();
        for (int i = 0; i < pixel_count; i++, ptr++)
        {
            if (ptr->a != 0xFF)
                return false;
        }
        return true;
    }

    /**
        @name Draw Image.
        if it's a 32bpp image, it must have been <span style='color:#FF0000'>premultipled</span>.
    */
    //@{
    /// draw image.
    static void Draw(HDC dc, POINT pt_on_dc, HBITMAP img)
    {
        CRect   rc(pt_on_dc, FCImageDrawDC::GetBitmapSize(img));
        Draw(dc, rc, img);
    }

    /// draw image.
    static void Draw(HDC dc, CRect dest, HBITMAP img, const RECT* rect_on_image = nullptr)
    {
        DIBSECTION   info = { 0 };
        if (::GetObject(img, sizeof(info), &info))
        {
            CRect   rc = (rect_on_image ? *rect_on_image : CRect(0, 0, info.dsBm.bmWidth, info.dsBm.bmHeight));

            if ((info.dsBmih.biBitCount == 32) && info.dsBm.bmBits) // is a DIB bitmap
            {
                BLENDFUNCTION   bf = { 0 };
                bf.BlendOp = AC_SRC_OVER;
                bf.SourceConstantAlpha = 255;
                bf.AlphaFormat = AC_SRC_ALPHA;
                ::GdiAlphaBlend(dc, dest.left, dest.top, dest.Width(), dest.Height(), FCImageDrawDC(img), rc.left, rc.top, rc.Width(), rc.Height(), bf);
            }
            else
            {
                int   old = SetStretchBltMode(dc, COLORONCOLOR);
                StretchBlt(dc, dest.left, dest.top, dest.Width(), dest.Height(), FCImageDrawDC(img), rc.left, rc.top, rc.Width(), rc.Height(), SRCCOPY);
                SetStretchBltMode(dc, old);
            }
        }
    }
    //@}

    /// @name Process Image.
    //@{
    /// get region of image, rect_on_image must inside image.
    static void GetRegion(const FCImage& img, CRect rect_on_image, FCImage& output)
    {
        CRect   rc;
        rc.IntersectRect(CRect(0, 0, img.Width(), img.Height()), rect_on_image);
        if (!rc.IsRectEmpty() &&
            (rc == rect_on_image) &&
            output.Create(rc.Width(), rc.Height(), img.ColorBits(), img.GetAttribute()))
        {
            int   copy_bytes = output.Width() * img.ColorBits() / 8;
            for (int y = 0; y < output.Height(); y++)
            {
                memcpy(output.GetBits(y), img.GetBits(rc.left, rc.top + y), copy_bytes);
            }
            return;
        }
        assert(false);
    }

    /// cover image.
    static void Cover(FCImage& bottom, const FCImage& top, POINT pt_on_bottom)
    {
        CRect   rect_top(pt_on_bottom, CSize(top.Width(), top.Height()));
        CRect   rc;
        rc.IntersectRect(CRect(0, 0, bottom.Width(), bottom.Height()), rect_top);
        if (!rc.IsRectEmpty() && (bottom.ColorBits() == top.ColorBits()) && (bottom.GetAttribute() == top.GetAttribute()))
        {
            for (int y = rc.top; y < rc.bottom; y++)
            {
                auto   dest = bottom.GetBits(rc.left, y);
                auto   src = top.GetBits(rc.left - pt_on_bottom.x, y - pt_on_bottom.y);
                memcpy(dest, src, rc.Width() * top.ColorBits() / 8);
            }
            return;
        }
        assert(false);
    }
    //@}
};
