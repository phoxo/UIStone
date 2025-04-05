#pragma once

class CWidgetItem
{
private:
    enum
    {
        MouseHover = 0x01,
        MouseHighlight = 0x02,
        WidgetVisible = 0x04,
        WidgetEnable = 0x08,
        WidgetCheck = 0x10,
        ReverseDrawOrder = 0x20,
    };

private:
    const int   m_id;
    int   m_style = 0;
    int   m_inner_status = WidgetVisible | WidgetEnable;
    CRect    m_rect_on_canvas;
    CString   m_tip;

public:
    CWidgetItem(int item_id = 0) : m_id(item_id) {}
    virtual ~CWidgetItem() {}

    int GetID() const { return m_id; }
    void SetTip(const CString& tip_text) { m_tip = tip_text; }
    auto& GetTip() const { return m_tip; }
    void SetRectOnCanvas(const CRect& rect_on_canvas) { m_rect_on_canvas = rect_on_canvas; }
    auto& GetRectOnCanvas() const { return m_rect_on_canvas; }
    // the top left point is (0,0)
    CRect GetItemClientRect() const { return CRect(CPoint(), m_rect_on_canvas.Size()); }
    void ModifyStyle(int remove, int add) { m_style = ((m_style & ~remove) | add); }
    int GetStyle() const { return m_style; }

    bool IsHighlight() const { return m_inner_status & MouseHighlight; }
    bool IsMouseHovering() const { return m_inner_status & MouseHover; }
    bool IsVisible() const { return m_inner_status & WidgetVisible; }
    bool IsEnable() const { return m_inner_status & WidgetEnable; }
    bool IsCheck() const { return m_inner_status & WidgetCheck; }
    bool IsDrawOrderReversed() const { return m_inner_status & ReverseDrawOrder; }
    void SetVisible(bool v) { v ? SetInnerStatus(0, WidgetVisible) : SetInnerStatus(WidgetVisible, 0); }
    void SetEnable(bool v) { v ? SetInnerStatus(0, WidgetEnable) : SetInnerStatus(WidgetEnable, 0); }
    void SetCheck(bool v) { v ? SetInnerStatus(0, WidgetCheck) : SetInnerStatus(WidgetCheck, 0); }
    void SetDrawOrderReversed() { SetInnerStatus(0, ReverseDrawOrder); }

protected:
    virtual void OnMouseEnter() {}
    virtual void OnMouseLeave() {}
    /// Draw item, the origin point (0,0) of DC has been moved to top left of item.
    virtual void OnDrawWidget(CDC& dc) = 0;
    virtual void OnRegisterTip(CToolTipCtrl& tip_ctrl, CWnd* parent_wnd, int tip_id, CRect rect_on_window)
    {
        if (!m_tip.IsEmpty())
        {
            tip_ctrl.AddTool(parent_wnd, m_tip, rect_on_window, tip_id);
        }
    }

private:
    void SetHighlight(bool v) { v ? SetInnerStatus(0, MouseHighlight) : SetInnerStatus(MouseHighlight, 0); }
    void SetHover(bool v) { v ? SetInnerStatus(0, MouseHover) : SetInnerStatus(MouseHover, 0); }
    void SetInnerStatus(int remove, int add) { m_inner_status = ((m_inner_status & ~remove) | add); }
    friend class CWidgetWindow;
};
