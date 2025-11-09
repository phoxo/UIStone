#pragma once

// DIB or DDB, 如果是32位色，必须Premultiplied alpha
class GDIBitmap
{
private:
    HBITMAP   m_bmp = nullptr;

public:
    ~GDIBitmap()
    {
        Delete();
    }

    operator HBITMAP() const { return m_bmp; }

    void CreateDDB(CSize image_size)
    {
        Delete();
        HDC   dc = GetDC(NULL);
        m_bmp = CreateCompatibleBitmap(dc, image_size.cx, image_size.cy);
        ReleaseDC(NULL, dc);
    }

    void CreateDDBFromDIB(const phoxo::Image& src, HBRUSH fill_background)
    {
        using namespace phoxo;

        if (src.ColorBits() == 32)
        {
            assert(src.IsPremultiplied());
        }

        CreateDDB(src.Size());
        BitmapHDC   dest_dc(m_bmp);
        if (fill_background)
        {
            ::FillRect(dest_dc, CRect(CPoint(), src.Size()), fill_background);
        }
        ImageDrawer::Draw(dest_dc, CPoint(0, 0), src);
    }

    void Delete()
    {
        if (m_bmp) { ::DeleteObject(m_bmp); }
        m_bmp = nullptr;
    }

    void Attach(HBITMAP new_bmp)
    {
        Delete();
        m_bmp = new_bmp;
    }

    HBITMAP Detach()
    {
        HBITMAP   t = m_bmp;
        m_bmp = nullptr;
        return t;
    }

    static bool IsDDB(HBITMAP bmp)
    {
        DIBSECTION   info{};
        ::GetObject(bmp, sizeof(info), &info);
        return info.dsBm.bmBits == nullptr;
    }
};
