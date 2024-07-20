#pragma once
UISTONE_BEGIN

template<class T>
struct GdipRectT : public T
{
    GdipRectT(const CRect& t) : T(t.left, t.top, t.Width(), t.Height())
    {
    }
};

typedef  GdipRectT<Gdiplus::Rect>  GdipRect;

UISTONE_END
