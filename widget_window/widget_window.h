#pragma once
#include "widget_layout.h"
#include "widget_item_bitmap_button.h"
#include "widget_item_text.h"
#include "widget_item_image.h"
#include "scrollbar.h"

class CWidgetWindow : public CWnd,
                      public ITrackMouseHover
{
private:
    CToolTipCtrl   m_tip_ctrl;
    std::unique_ptr<CWidgetLayout>   m_layout;
    std::deque<std::unique_ptr<CWidgetItem>>   m_child_widget;
    CFont   m_font;
    internal::Scrollbar   m_scrollbar;
    CWidgetItem   * m_highlight = nullptr; // 拖拽消息循环中，可能被别的事件激活Reload，item已经被删除了

public:
    CWidgetWindow() : m_scrollbar(*this) {}
    virtual ~CWidgetWindow()
    {
        m_tip_ctrl.DestroyWindow();
        DestroyWindow();
    }

    void Create(const CRect& rect_on_parent, CWnd* parent_wnd, UINT nID, DWORD wnd_style = WS_VISIBLE)
    {
        CWnd::Create(NULL, NULL, WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN | wnd_style, rect_on_parent, parent_wnd, nID);
    }
    void SetWidgetLayout(CWidgetLayout* widget_layout) { m_layout.reset(widget_layout); }
    void SetFont(int point_size, PCWSTR facename) { m_font.CreatePointFont(point_size, facename); }
    auto& GetFont() const { return m_font; }

    void AddWidget(CWidgetItem* item, int add_index = -1);
    CWidgetItem* FindWidgetByID(int id) const;
    CWidgetItem* GetWidgetByIndex(int index) const;
    void DeleteAllWidget();
    void DeleteWidgetByID(int id);
    void DeleteWidgetByIndex(int index);
    CWidgetItem* ReleaseWidgetOwnership(int id);
    void LayoutWidget();
    CWidgetItem* ClickHitTest(CPoint pt_on_window, bool scan_hidden = false) const;
    auto& GetAllWidgets() const { return m_child_widget; }

    CRect CanvasToWindow(const CRect& rect_on_canvas) const { return rect_on_canvas - m_scrollbar.GetPos(); }

protected:
    // coordinate conversion
    CRect GetItemRectOnWindow(const CWidgetItem& item) const { return CanvasToWindow(item.GetRectOnCanvas()); }

    void RegisterToolTip();

    virtual void OnCreateTooltip(CToolTipCtrl& tip_ctrl);
    /// you can think of dc as window client dc.
    virtual void DrawWidgetWindowBack(CDC& dc, CRect update_on_dc) {}
    virtual bool OnHighlightBegin(CWidgetItem* highlight) { return false; }
    virtual void OnClickWidget(CWidgetItem& widget) {}
    virtual void OnMsgLButtonDown(CPoint pt_on_window);
    virtual void OnMsgLButtonDoubleClick(CPoint pt_on_window) { OnMsgLButtonDown(pt_on_window); }
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual LRESULT WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    void OnMsgPaint(CDC& paint_dc, CRect update_rect);
    void OnMsgScroll(int bar, int event);
    void OnMsgMouseMove(CPoint pt_on_window);
    void OnHighlightMouseMove(CPoint pt_on_window);
    void OnHighlightEnd(CPoint pt_on_window);
    virtual void OnMouse_LeaveWnd() { OnMsgMouseMove(CPoint(-0xFFFF, -0xFFFF)); }
    int FindWidgetIndex(int id) const;
    bool IsDrawOrderReversed() const { return m_child_widget.size() && m_child_widget.front()->IsDrawOrderReversed(); }

    struct AutoClearHighlight
    {
        CWidgetItem   *& p_;
        AutoClearHighlight(CWidgetItem*& p) : p_(p) {}
        ~AutoClearHighlight() { p_ = nullptr; }
    };

    template <typename T>
    void PaintAllWidgets(CDC& dc, const T& vec, const CRect& update_rect)
    {
        for (CSize sbpos = m_scrollbar.GetPos(); auto& iter : vec)
        {
            CRect   widget_rect = iter->GetRectOnCanvas() - sbpos; // convert to rect on window
            if (iter->IsVisible() && CRect().IntersectRect(widget_rect, update_rect))
            {
                // for every item, the origin of DC is point (0,0)
                CPoint   old = dc.OffsetViewportOrg(widget_rect.left, widget_rect.top);
                iter->OnDrawWidget(dc);
                dc.SetViewportOrg(old);
            }
        }
    }
};
//-------------------------------------------------------------------------------------
inline void CWidgetWindow::AddWidget(CWidgetItem* item_src, int add_index)
{
    std::unique_ptr<CWidgetItem>   item(item_src);
    if (!item) return;
    if (add_index == -1)
    {
        m_child_widget.push_back(std::move(item));
    }
    else
    {
        if ((add_index >= 0) && (add_index <= (int)m_child_widget.size()))
        {
            m_child_widget.insert(m_child_widget.begin() + add_index, std::move(item));
        }
        else { ASSERT(false); }
    }
}

inline int CWidgetWindow::FindWidgetIndex(int id) const
{
    for (size_t i = 0; i < m_child_widget.size(); i++)
    {
        if (m_child_widget[i]->GetID() == id)
            return (int)i;
    }
    return -1;
}

inline CWidgetItem* CWidgetWindow::FindWidgetByID(int id) const
{
    int   pos = FindWidgetIndex(id);
    return (pos != -1) ? m_child_widget[pos].get() : nullptr;
}

inline CWidgetItem* CWidgetWindow::GetWidgetByIndex(int index) const
{
    bool   ok = FCHelper::IsValidIndex(m_child_widget, index); ASSERT(ok);
    return ok ? m_child_widget[index].get() : nullptr;
}

inline void CWidgetWindow::DeleteAllWidget()
{
    m_child_widget.clear();
    m_tip_ctrl.DestroyWindow();
    m_highlight = nullptr;
}

inline void CWidgetWindow::DeleteWidgetByID(int id)
{
    DeleteWidgetByIndex(FindWidgetIndex(id));
}

inline void CWidgetWindow::DeleteWidgetByIndex(int index)
{
    if (FCHelper::IsValidIndex(m_child_widget, index))
    {
        m_child_widget.erase(m_child_widget.begin() + index);
    }
}

inline CWidgetItem* CWidgetWindow::ReleaseWidgetOwnership(int id)
{
    if (int pos = FindWidgetIndex(id); pos != -1)
    {
        auto   t = m_child_widget[pos].release();
        DeleteWidgetByIndex(pos);
        return t;
    }
    return nullptr;
}

inline void CWidgetWindow::LayoutWidget()
{
    if (!m_layout || !m_hWnd) { ASSERT(false); return; }

    m_layout->LayoutWidget(*this, m_child_widget);
    RegisterToolTip();
    Invalidate();
}

inline CWidgetItem* CWidgetWindow::ClickHitTest(CPoint pt_on_window, bool scan_hidden) const
{
    CPoint   pt = pt_on_window + m_scrollbar.GetPos();
    for (auto& iter : m_child_widget)
    {
        if (iter->GetRectOnCanvas().PtInRect(pt) && iter->IsEnable())
        {
            if (scan_hidden || iter->IsVisible())
                return iter.get();
        }
    }
    return nullptr;
}

inline void CWidgetWindow::RegisterToolTip()
{
    if (!m_hWnd) { ASSERT(false); return; }

    CRect   wnd_rect = FCWnd::GetClientRect(*this);
    CSize   sbpos = m_scrollbar.GetPos();

    m_tip_ctrl.DestroyWindow();
    int   tip_id = 100;
    for (auto& iter : m_child_widget)
    {
        if (iter->IsVisible() && !iter->m_tip.IsEmpty())
        {
            CRect   rect_on_wnd = iter->GetRectOnCanvas() - sbpos;
            if (CRect().IntersectRect(rect_on_wnd, wnd_rect))
            {
                if (!m_tip_ctrl)
                    OnCreateTooltip(m_tip_ctrl);
                iter->OnRegisterTip(m_tip_ctrl, this, tip_id++, rect_on_wnd);
            }
        }
    }
}

inline void CWidgetWindow::OnCreateTooltip(CToolTipCtrl& tip_ctrl)
{
    m_tip_ctrl.Create(this, TTS_ALWAYSTIP);
    m_tip_ctrl.Activate(TRUE);
}

inline void CWidgetWindow::OnHighlightEnd(CPoint pt_on_window)
{
    auto   now_item = ClickHitTest(pt_on_window);
    if (now_item && (now_item == m_highlight))
    {
        OnClickWidget(*now_item);
    }
}

inline BOOL CWidgetWindow::PreTranslateMessage(MSG* pMsg)
{
    if (m_tip_ctrl)
        m_tip_ctrl.RelayEvent(pMsg);
    return __super::PreTranslateMessage(pMsg);
}

inline void CWidgetWindow::OnMsgPaint(CDC& paint_dc, CRect update_rect)
{
    CBitmap   bmp;
    if (bmp.CreateCompatibleBitmap(&paint_dc, update_rect.Width(), update_rect.Height()))
    {
        BitmapHDC   auto_bmp_selected(bmp); // 析构自动选出
        CDC   & mem_dc = *CDC::FromHandle(auto_bmp_selected);
        SelectObject(mem_dc, m_font.m_hObject ? m_font.m_hObject : FontManager::GetDefaultFont());

        mem_dc.SetViewportOrg(-update_rect.TopLeft()); // 原点移到窗口左上角
        DrawWidgetWindowBack(mem_dc, update_rect);

        if (IsDrawOrderReversed())
            PaintAllWidgets(mem_dc, std::views::reverse(m_child_widget), update_rect);
        else
            PaintAllWidgets(mem_dc, std::views::all(m_child_widget), update_rect);

        ::BitBlt(paint_dc, update_rect.left, update_rect.top, update_rect.Width(), update_rect.Height(), mem_dc, update_rect.left, update_rect.top, SRCCOPY);
    }
}

inline void CWidgetWindow::OnMsgScroll(int bar, int event)
{
    if (!m_child_widget.empty())
    {
        m_scrollbar.OnMsgScroll(bar, event, m_child_widget[0]->GetRectOnCanvas().Size());
        Invalidate();
        RegisterToolTip();
    }
}

inline void CWidgetWindow::OnMsgLButtonDown(CPoint pt_on_window)
{
    AutoClearHighlight   clear_highlight(m_highlight);
    Invalidate();
    OnMsgMouseMove(pt_on_window);
    m_highlight = ClickHitTest(pt_on_window);
    if (!m_highlight)
        return;
    if (OnHighlightBegin(m_highlight)) // 保证不为NULL
        return;

    SetCapture();
    if (m_highlight) { m_highlight->SetHighlight(true); }
    for (;;)
    {
        MSG   msg = {};
        ::GetMessage(&msg, NULL, 0, 0);
        if (::GetCapture() != m_hWnd) // 很难复现，被别的弹窗夺走焦点
        {
            ::DispatchMessage(&msg); // 此刻什么消息不知道，可能是WM_MOUSEMOVE, 0x0060，或者我们自己定义的消息WM_APP+xxx
            goto capture_over; // <== Exit.1
        }

        switch (msg.message)
        {
        case WM_MOUSEMOVE:
            OnHighlightMouseMove(msg.lParam);
            break;

        case WM_LBUTTONUP:
            ReleaseCapture();
            OnHighlightEnd(msg.lParam);
            goto capture_over; // <== Exit.2

        default: // just dispatch rest of the messages
            ::DispatchMessage(&msg);
            break;
        }
    }
capture_over:
    if (m_highlight) { m_highlight->SetHighlight(false); }
    Invalidate();
}

inline void CWidgetWindow::OnMsgMouseMove(CPoint pt_on_window)
{
    CWidgetItem   *last_hover = NULL, *now_hover = NULL;
    for (CPoint pt_canvas = pt_on_window + m_scrollbar.GetPos(); auto& iter : m_child_widget)
    {
        if (iter->IsMouseHovering())
            last_hover = iter.get();
        if (!now_hover && iter->GetRectOnCanvas().PtInRect(pt_canvas) && iter->IsVisible() && iter->IsEnable()) // !now_hover for find FIRST
            now_hover = iter.get();
    }

    if (now_hover != last_hover)
    {
        if (last_hover)
        {
            last_hover->SetHover(false);
            last_hover->OnMouseLeave();
            InvalidateRect(GetItemRectOnWindow(*last_hover));
        }
        if (now_hover)
        {
            now_hover->SetHover(true);
            now_hover->OnMouseEnter();
            InvalidateRect(GetItemRectOnWindow(*now_hover));
        }
    }
}

inline void CWidgetWindow::OnHighlightMouseMove(CPoint pt_on_window)
{
    if (!m_highlight)
        return;
    bool   hovering = (ClickHitTest(pt_on_window) == m_highlight);
    bool   old_hover = m_highlight->IsMouseHovering();
    if (hovering != old_hover)
    {
        m_highlight->SetHover(hovering);
        InvalidateRect(GetItemRectOnWindow(*m_highlight));
    }
}

inline LRESULT CWidgetWindow::WindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    ITrackMouseHover::FilterMouseMessage(m_hWnd, msg);
    switch (msg)
    {
    case WM_SIZE:
        LayoutWidget();
        break;

    case WM_ERASEBKGND: return 1;

    case WM_LBUTTONDOWN:
        OnMsgLButtonDown(lParam);
        return 0;

    case WM_LBUTTONDBLCLK:
        OnMsgLButtonDoubleClick(lParam);
        return 0;

    case WM_MOUSEMOVE:
        OnMsgMouseMove(lParam);
        return 0;

    case WM_PAINT:
        if (CPaintDC dc(this); !IsRectEmpty(&dc.m_ps.rcPaint))
            OnMsgPaint(dc, dc.m_ps.rcPaint);
        return 0;

    case WM_HSCROLL:
    case WM_VSCROLL:
        OnMsgScroll((msg == WM_VSCROLL) ? SB_VERT : SB_HORZ, LOWORD(wParam));
        return 0;

    case WM_MOUSEWHEEL:
        if (m_scrollbar.OnMsgMouseWheel(wParam, lParam))
            return 0;
        break;
    }
    return __super::WindowProc(msg, wParam, lParam);
}
