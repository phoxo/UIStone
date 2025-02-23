#pragma once

template<class T>
struct GdipRectT : public T
{
    GdipRectT(const CRect& t) : T(t.left, t.top, t.Width(), t.Height())
    {
    }
};

using GdipRect = GdipRectT<Gdiplus::Rect>;
