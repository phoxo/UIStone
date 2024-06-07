#pragma once
#include "widget_layout.h"
#include "widget_item_bitmap_button.h"
#include "widget_item_text.h"
#include "widget_item_image.h"
_UISTONE_BEGIN

class CWidgetWindow : public CWnd,
                      public FCTrackMouseHover
{
public:
    struct ScrollPace
    {
        int   _page = 0; // use first item's size
        int   _line = 5;
    };

private:
    CToolTipCtrl   m_tip_ctrl;
    std::unique_ptr<CWidgetLayout>   m_layout;
    std::deque<std::unique_ptr<CWidgetItem>>   m_child_widget;
    CFont   m_font;
    ScrollPace   m_scroll_pace;

public:
    virtual ~CWidgetWindow()
    {
        m_tip_ctrl.DestroyWindow();
        DestroyWindow();
    }

    void Create(CRect rect_on_parent, CWnd* parent_wnd, UINT nID, DWORD wnd_style = WS_VISIBLE)
    {
        CWnd::Create(NULL, NULL, WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN | wnd_style, rect_on_parent, parent_wnd, nID);
    }
    void SetWidgetLayout(CWidgetLayout* widget_layout) { m_layout.reset(widget_layout); }
    void SetFont(int point_size, PCWSTR facename) { m_font.CreatePointFont(point_size, facename); }
    const CFont& GetFont() const { return m_font; }

    void AddWidget(CWidgetItem* item, int add_index = -1);
    CWidgetItem* FindWidgetByID(int id) const;
    void DeleteAllWidget();
    void DeleteWidgetByID(int id);
    CWidgetItem* ReleaseWidgetOwnership(int id);
    void LayoutWidget();
    CWidgetItem* ClickHitTest(CPoint pt_on_window) const;
    int GetWidgetCount() const { return (int)m_child_widget.size(); }

protected:
    // coordinate conversion
    CSize GetScrollPosition() const;
    CRect GetItemRectOnWindow(const CWidgetItem& item) const;

    void RegisterToolTip();
    void ExclusiveCheck(CWidgetItem* widget);

    virtual void OnCreateTooltip(CToolTipCtrl& tip_ctrl);
    /// you can think of dc as window client dc.
    virtual void DrawWidgetWindowBack(CDC& dc, CRect update_on_dc) {}
    virtual BOOL OnHighlightBegin(CWidgetItem* highlight) { return FALSE; }
    virtual void OnHighlightEnd(CWidgetItem* highlight, CPoint pt_on_window, BOOL lbutton_up);
    virtual void OnClickWidget(CWidgetItem& widget) {}
    virtual void OnMsgLButtonDown(CPoint pt_on_window);
    virtual void OnMsgLButtonDoubleClick(CPoint pt_on_window) { OnMsgLButtonDown(pt_on_window); }
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual LRESULT WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    void OnMsgPaint();
    void OnMsgVScroll(int sb_code);
    void OnMsgMouseMove(CPoint pt_on_window);
    void OnHighlightMouseMove(CWidgetItem& highlight, CPoint pt_on_window);
    virtual void OnMouse_LeaveWnd() { OnMsgMouseMove(CPoint(-0xFFFF, -0xFFFF)); }
    int FindWidgetIndex(int id) const;
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

inline void CWidgetWindow::DeleteAllWidget()
{
    m_child_widget.clear();
    m_tip_ctrl.DestroyWindow();
}

inline void CWidgetWindow::DeleteWidgetByID(int id)
{
    int   pos = FindWidgetIndex(id);
    if (pos != -1)
    {
        m_child_widget.erase(m_child_widget.begin() + pos);
    }
}

inline CWidgetItem* CWidgetWindow::ReleaseWidgetOwnership(int id)
{
    int   pos = FindWidgetIndex(id);
    if (pos != -1)
    {
        auto   t = m_child_widget[pos].release();
        m_child_widget.erase(m_child_widget.begin() + pos);
        return t;
    }
    return nullptr;
}

inline void CWidgetWindow::LayoutWidget()
{
    if (!m_layout.get() || !m_hWnd) { ASSERT(false); return; }

    m_layout->LayoutWidget(*this, m_child_widget);
    RegisterToolTip();
    Invalidate();
}

inline CWidgetItem* CWidgetWindow::ClickHitTest(CPoint pt_on_window) const
{
    CPoint   pt = pt_on_window + GetScrollPosition();
    for (auto& iter : m_child_widget)
    {
        if (iter->GetRectOnCanvas().PtInRect(pt) && iter->IsVisible() && iter->IsEnable())
            return iter.get();
    }
    return NULL;
}

inline CSize CWidgetWindow::GetScrollPosition() const
{
    int   y = FCWnd::IsVScrollVisible(*this) ? GetScrollPos(SB_VERT) : 0;
    return CSize(0, y);
}

inline CRect CWidgetWindow::GetItemRectOnWindow(const CWidgetItem& item) const
{
    return item.GetRectOnCanvas() - GetScrollPosition();
}

inline void CWidgetWindow::RegisterToolTip()
{
    if (!m_hWnd) { ASSERT(false); return; }

    CRect   wnd_rect = FCWnd::GetClientRect(*this);
    CSize   sbpos = GetScrollPosition();

    m_tip_ctrl.DestroyWindow();
    int   tip_id = 100;
    for (auto& iter : m_child_widget)
    {
        if (iter->IsVisible())
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

inline void CWidgetWindow::ExclusiveCheck(CWidgetItem* widget)
{
    for (auto& iter : m_child_widget)
    {
        iter->SetCheck(iter.get() == widget);
    }
}

inline void CWidgetWindow::OnCreateTooltip(CToolTipCtrl& tip_ctrl)
{
    m_tip_ctrl.Create(this, TTS_ALWAYSTIP);
    m_tip_ctrl.Activate(TRUE);
}

inline void CWidgetWindow::OnHighlightEnd(CWidgetItem* highlight, CPoint pt_on_window, BOOL lbutton_up)
{
    CWidgetItem   * now_item = ClickHitTest(pt_on_window);
    if (lbutton_up && now_item && (now_item == highlight))
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

inline void CWidgetWindow::OnMsgPaint()
{
    CPaintDC   paint_dc(this);
    CRect   update_rect = paint_dc.m_ps.rcPaint;
    if (update_rect.IsRectEmpty())
        return;

    CBitmap   bmp;
    if (bmp.CreateCompatibleBitmap(&paint_dc, update_rect.Width(), update_rect.Height()))
    {
        FCImageDrawDC   auto_bmp_selected(bmp); // 析构自动选出
        CDC   * mem_dc = CDC::FromHandle(auto_bmp_selected);
        SelectObject(*mem_dc, m_font.m_hObject ? m_font.m_hObject : FCFontManager::GetDefaultFont());

        mem_dc->SetViewportOrg(-update_rect.TopLeft()); // 原点移到窗口左上角
        DrawWidgetWindowBack(*mem_dc, update_rect);

        CSize   sbpos = GetScrollPosition();
        for (auto& iter : m_child_widget)
        {
            if (iter->IsVisible())
            {
                CRect   widget_rect = iter->GetRectOnCanvas() - sbpos; // convert to rect on window
                if (CRect().IntersectRect(widget_rect, update_rect))
                {
                    // for every item, the origin of DC is point (0,0)
                    CPoint   old_origin = mem_dc->OffsetViewportOrg(widget_rect.left, widget_rect.top);
                    iter->OnDrawWidget(*mem_dc);
                    mem_dc->SetViewportOrg(old_origin);
                }
            }
        }
        paint_dc.BitBlt(update_rect.left, update_rect.top, update_rect.Width(), update_rect.Height(), mem_dc, update_rect.left, update_rect.top, SRCCOPY);
    }
}

inline void CWidgetWindow::OnMsgVScroll(int sb_code)
{
    if (m_child_widget.empty() || !FCWnd::IsVScrollVisible(*this))
        return;

    ScrollPace   pace = m_scroll_pace;
    if (!pace._page)
        pace._page = m_child_widget[0]->GetRectOnCanvas().Height();

    int   pos = FCWnd::GetScrollbarPos(*this, sb_code);
    switch (sb_code)
    {
    case SB_PAGEUP: pos -= pace._page; break;
    case SB_LINEUP: pos -= pace._line; break;
    case SB_PAGEDOWN: pos += pace._page; break;
    case SB_LINEDOWN: pos += pace._line; break;
    }

    SCROLLINFO   si = { 0, SIF_POS };
    si.nPos = pos;
    SetScrollInfo(SB_VERT, &si, TRUE);
    Invalidate();
    RegisterToolTip();
}

inline void CWidgetWindow::OnMsgLButtonDown(CPoint pt_on_window)
{
    Invalidate();
    OnMsgMouseMove(pt_on_window);
    CWidgetItem   * highlight = ClickHitTest(pt_on_window);
    if (OnHighlightBegin(highlight))
        return;

    SetCapture();
    if (highlight) { highlight->SetHighlight(TRUE); }
    for (;;)
    {
        MSG   msg = { 0 };
        ::GetMessage(&msg, NULL, 0, 0);
        if (::GetCapture() != m_hWnd)
        {
            OnHighlightEnd(highlight, msg.lParam, FALSE);
            goto capture_over; // <== Exit.1
        }

        switch (msg.message)
        {
        case WM_MOUSEMOVE:
            if (highlight)
                OnHighlightMouseMove(*highlight, msg.lParam);
            break;

        case WM_LBUTTONUP:
            ReleaseCapture();
            OnHighlightEnd(highlight, msg.lParam, TRUE);
            goto capture_over; // <== Exit.2

        default: // just dispatch rest of the messages
            ::DispatchMessage(&msg);
            break;
        }
    }
capture_over:
    if (highlight) { highlight->SetHighlight(FALSE); }
    Invalidate();
}

inline void CWidgetWindow::OnMsgMouseMove(CPoint pt_on_window)
{
    CPoint   pt_canvas = pt_on_window + GetScrollPosition();
    CWidgetItem   *last_hover = NULL, *now_hover = NULL;
    for (auto& iter : m_child_widget)
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
            last_hover->SetHover(FALSE);
            last_hover->OnMouseLeave();
            InvalidateRect(GetItemRectOnWindow(*last_hover));
        }
        if (now_hover)
        {
            now_hover->SetHover(TRUE);
            now_hover->OnMouseEnter();
            InvalidateRect(GetItemRectOnWindow(*now_hover));
        }
    }
}

inline void CWidgetWindow::OnHighlightMouseMove(CWidgetItem& highlight, CPoint pt_on_window)
{
    BOOL   hovering = (ClickHitTest(pt_on_window) == &highlight);
    BOOL   old_hover = highlight.IsMouseHovering();
    if ((hovering && !old_hover) || (!hovering && old_hover))
    {
        highlight.SetHover(hovering);
        InvalidateRect(GetItemRectOnWindow(highlight));
    }
}

inline LRESULT CWidgetWindow::WindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    FCTrackMouseHover::FilterMouseMessage(m_hWnd, msg);

    switch (msg)
    {
    case WM_SIZE:
        LayoutWidget();
        break;

    case WM_ERASEBKGND: return TRUE;

    case WM_LBUTTONDOWN:
        OnMsgLButtonDown(lParam);
        return 0;

    case WM_LBUTTONDBLCLK:
        OnMsgLButtonDoubleClick(lParam);
        return 0;

    case WM_MOUSEMOVE:
        OnMsgMouseMove(lParam);
        break;

    case WM_PAINT:
        OnMsgPaint();
        return 0;

    case WM_VSCROLL:
        OnMsgVScroll(LOWORD(wParam));
        break;

    case WM_MOUSEWHEEL:
        if (FCWnd::IsVScrollVisible(*this))
        {
            int   page = (GET_WHEEL_DELTA_WPARAM(wParam) > 0) ? SB_PAGEUP : SB_PAGEDOWN;
            PostMessage(WM_VSCROLL, MAKEWPARAM(page, 0));
            CPoint   pt(lParam);
            ScreenToClient(&pt);
            PostMessage(WM_MOUSEMOVE, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
        break;
    }
    return __super::WindowProc(msg, wParam, lParam);
}

_UISTONE_END
