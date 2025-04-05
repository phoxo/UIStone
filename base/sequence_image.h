#pragma once

// images arranged horizontally one by one
class SequenceImage
{
private:
    IWICBitmapPtr   m_original;
    int   m_count = 0; // 横向排列，如果为零表示每个大小一样的正方形
    int   m_row = 1;
    CSize   m_src_item;
    CSize   m_dest_each;
    int   m_image_designed_for_dpi;

public:
    SequenceImage(PCWSTR filepath, int image_designed_for_dpi = 2 * USER_DEFAULT_SCREEN_DPI, int count = 0) : SequenceImage(WIC::CreateStreamFromFileNoLock(filepath), image_designed_for_dpi, count)
    {
    }

    SequenceImage(IStream* sp, int image_designed_for_dpi = 2 * USER_DEFAULT_SCREEN_DPI, int count = 0)
    {
        m_original = WIC::LoadPlainImageFromStream(sp, WICNormal32bpp);
        m_count = count;
        m_image_designed_for_dpi = image_designed_for_dpi;
    }

    void SetRowCount(int row)
    {
        m_row = (std::max)(1, row);
    }

    CSize GetSizeOfEachImage() const
    {
        return m_dest_each;
    }

    void Load(phoxo::Image& output, REFWICPixelFormatGUID output_format)
    {
        CSize   image_size = WIC::GetBitmapSize(m_original);
        if (!image_size.cy)
            return;

        CalcInput(image_size);

        CRect   oldrect(CPoint(), m_src_item);
        CRect   newrect(CPoint(), m_dest_each);
        for (int y = 0; y < m_row; y++)
        {
            for (int i = 0; i < m_count; i++)
            {
                MapRectWithDpi(output, oldrect, newrect, output_format);
            }
            oldrect.MoveToX(0); oldrect.OffsetRect(0, oldrect.Height());
            newrect.MoveToX(0); newrect.OffsetRect(0, newrect.Height());
        }
    }

    static void LoadFile(PCWSTR filepath, phoxo::Image& output, REFWICPixelFormatGUID output_format, int count = 0)
    {
        SequenceImage   t(filepath, 2 * USER_DEFAULT_SCREEN_DPI, count);
        t.Load(output, output_format);
    }

private:
    void CalcInput(CSize original_size)
    {
        m_src_item.cy = original_size.cy / m_row;
        if (m_count)
        {
            m_src_item.cx = original_size.cx / m_count;
        }
        else
        {
            m_src_item.cx = m_src_item.cy; // 图片是正方形
            m_count = original_size.cx / m_src_item.cx;
        }

        m_dest_each.cx = DPICalculator::Cast(m_src_item.cx, m_image_designed_for_dpi);
        m_dest_each.cy = DPICalculator::Cast(m_src_item.cy, m_image_designed_for_dpi);
    }

    // get rect of original scale to new image
    IWICBitmapSourcePtr ScaleRegion(CRect rc)
    {
        IWICBitmapPtr   rgn;
        WIC::g_factory->CreateBitmapFromSourceRect(m_original, rc.left, rc.top, rc.Width(), rc.Height(), &rgn);
        return WIC::ScaleBitmap(rgn, m_dest_each);
    }

    void MapRectWithDpi(phoxo::Image& output, CRect& oldrect, CRect& newrect, REFWICPixelFormatGUID output_format)
    {
        auto   t = phoxo::ImageHandler::Make(ScaleRegion(oldrect), output_format);
        if (!output)
        {
            output.Create(m_dest_each.cx * m_count, m_dest_each.cy * m_row, t.ColorBits(), t.GetAttribute());
        }
        ImageHandler::Cover(output, t, newrect.TopLeft());

        oldrect.OffsetRect(oldrect.Width(), 0);
        newrect.OffsetRect(newrect.Width(), 0);
    }
};
