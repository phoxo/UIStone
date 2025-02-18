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
    virtual void OnDrawWidget(CDC& dc)
    {
        CSize   sz = phoxo::Utils::GetBitmapSize(m_img);
        CRect   src(0, 0, sz.cx / 3, sz.cy);
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

    void SizeToBitmap()
    {
        CSize   sz = phoxo::Utils::GetBitmapSize(m_img);
        CRect   rc = GetRectOnCanvas();
        rc.right = rc.left + (sz.cx / 3);
        rc.bottom = rc.top + sz.cy;
        SetRectOnCanvas(rc);
    }
};
