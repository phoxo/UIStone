#pragma once

class CImagePropertyGdiplus
{
private:
    std::vector<BYTE>   m_buf;
    std::vector<Gdiplus::PropertyItem>   m_items;

public:
    int   m_save_jpeg_quality = 0; // 1 <= n <= 100
    int   m_dpi = 0;

public:
    void Load(Gdiplus::Bitmap& bmp)
    {
        m_dpi = (int)(bmp.GetHorizontalResolution() + 0.5);

        UINT   buf_size = 0, prop_num = 0;
        bmp.GetPropertySize(&buf_size, &prop_num);
        if (!buf_size || !prop_num)
            return;

        m_buf.resize(buf_size);
        m_items.reserve(prop_num);

        auto   itemvec = (Gdiplus::PropertyItem*)m_buf.data();
        bmp.GetAllPropertyItems(buf_size, prop_num, itemvec);

        for (UINT i = 0; i < prop_num; i++)
        {
            m_items.push_back(itemvec[i]);
        }
    }

    const Gdiplus::PropertyItem* FindProperty(PROPID prop_id) const
    {
        for (auto& iter : m_items)
        {
            if (iter.id == prop_id)
                return &iter;
        }
        return NULL;
    }

    void AddAllProperty(Gdiplus::Bitmap& bmp) const
    {
        if (m_dpi)
        {
            bmp.SetResolution((float)m_dpi, (float)m_dpi);
        }

        for (auto& iter : m_items)
        {
            bmp.SetPropertyItem(&iter);
        }
    }

    void ClearEXIF()
    {
        m_items.clear();
    }
};
