#pragma once

// DIB or DDB, �����32λɫ������Premultiplied alpha
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
        if (src.ColorBits() == 32)
        {
            assert(src.IsPremultiplied());
        }

        CreateDDB(CSize(src.Width(), src.Height()));
        BitmapHDC   dest_dc(m_bmp, fill_background);
        ImageHandler::Draw(dest_dc, CPoint(0, 0), src);
    }

    static HBITMAP CreateDDB(const phoxo::Image& img, HBRUSH fill_background)
    {
        GDIBitmap   ddb;
        ddb.CreateDDBFromDIB(img, fill_background);
        return ddb.Detach();
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
};
