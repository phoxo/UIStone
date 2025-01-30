#pragma once

class CWidgetItemImage : public CWidgetItem
{
private:
    CGDIBitmap   m_img;

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
        assert(m_img);
        ImageHandler::Draw(dc, GetItemClientRect(), m_img);
    }

    void SizeToBitmap()
    {
        CSize   sz = phoxo::Utils::GetBitmapSize(m_img);
        CRect   rc = GetRectOnCanvas();
        rc.BottomRight() = rc.TopLeft() + sz;
        SetRectOnCanvas(rc);
    }
};
