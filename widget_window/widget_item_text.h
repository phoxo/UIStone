#pragma once

class CWidgetItemText : public CWidgetItem
{
private:
    CString   m_text;
    std::shared_ptr<CFont>   m_font;
    UINT   m_format = DT_VCENTER | DT_SINGLELINE | DT_CENTER;
    COLORREF   m_color = 0;

public:
    using CWidgetItem::CWidgetItem;

    void SetText(LPCTSTR text) { m_text = text; }
    void SetFont(const std::shared_ptr<CFont>& font) { m_font = font; }
    void SetFormat(UINT fmt) { m_format = fmt; }
    void SetColor(COLORREF cr) { m_color = cr; }

protected:
    void OnDrawWidget(CDC& dc) override
    {
        if (!m_font) { ASSERT(false); return; }

        auto   old_font = SelectObject(dc, *m_font);
        dc.SetTextColor(m_color);
        dc.DrawText(m_text, GetItemClientRect(), m_format);
        SelectObject(dc, old_font);
    }
};
