#pragma once

class CWidgetItemBitmapButton : public CWidgetItem
{
private:
    GDIBitmap   m_img;

public:
    using CWidgetItem::CWidgetItem;

    void AttachBitmap(HBITMAP bmp)
    {
        m_img.Attach(bmp);
        SizeToBitmap();
    }

protected:
    void OnDrawWidget(CDC& dc) override
    {
        CRect   src(CPoint(), GetSliceSize());
        src.OffsetRect(QueryImageIndex() * src.Width(), 0);
        CRect   dest(CPoint(), src.Size());
        ImageHandler::Draw(dc, dest, m_img, src);
    }

private:
    int QueryImageIndex() const
    {
        if (IsHighlight())
            return 2;
        if (IsMouseHovering())
            return 1;
        return 0;
    }

    CSize GetSliceSize() const
    {
        CSize   sz = phoxo::Utils::GetBitmapSize(m_img);
        return CSize(sz.cx / 3, sz.cy);
    }

    void SizeToBitmap()
    {
        CRect   rc = GetRectOnCanvas();
        rc = CRect(rc.TopLeft(), GetSliceSize());
        SetRectOnCanvas(rc);
    }
};
