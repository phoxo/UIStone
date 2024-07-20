#pragma once

_IMSTONE_PRIVATE_BEGIN

class Scrollbar
{
private:
    CWnd   & m_host;
    int   m_page = 0; // use first item's size
    int   m_line = 5;

public:
    Scrollbar(CWnd& host) : m_host(host) {}

    CPoint GetPos() const
    {
        CPoint   pt;
        DWORD   style = m_host.GetStyle();
        if (style & WS_VSCROLL) { pt.y = m_host.GetScrollPos(SB_VERT); }
        if (style & WS_HSCROLL) { pt.x = m_host.GetScrollPos(SB_HORZ); }
        return pt;
    }

    bool OnMsgMouseWheel(WPARAM wParam, LPARAM lParam) const
    {
        UINT   msg = 0;
        DWORD   style = m_host.GetStyle();
        if (style & WS_HSCROLL) { msg = WM_HSCROLL; }
        if (style & WS_VSCROLL) { msg = WM_VSCROLL; } // 同时存在会只滚动Vertical
        if (msg)
        {
            int   page = (GET_WHEEL_DELTA_WPARAM(wParam) > 0) ? SB_PAGEUP : SB_PAGEDOWN;
            m_host.PostMessage(msg, MAKEWPARAM(page, 0));
            CPoint   pt(lParam);
            m_host.ScreenToClient(&pt);
            m_host.PostMessage(WM_MOUSEMOVE, wParam, MAKELPARAM(pt.x, pt.y));
            return true;
        }
        return false;
    }

    void OnMsgScroll(int bar, int event, SIZE first_item_size)
    {
        if (!m_page)
            m_page = ((bar == SB_VERT) ? first_item_size.cy : first_item_size.cx);

        int   pos = CallGetScrollPos(bar, event);
        switch (event)
        {
            case SB_PAGEUP: pos -= m_page; break;
            case SB_LINEUP: pos -= m_line; break;
            case SB_PAGEDOWN: pos += m_page; break;
            case SB_LINEDOWN: pos += m_line; break;
        }

        SCROLLINFO   si = { 0, SIF_POS };
        si.nPos = pos;
        m_host.SetScrollInfo(bar, &si, TRUE);
    }

private:
    int CallGetScrollPos(int bar, int event) const
    {
        SCROLLINFO   si = { 0 };
        m_host.GetScrollInfo(bar, &si);
        return ((event == SB_THUMBPOSITION) || (event == SB_THUMBTRACK)) ? si.nTrackPos : si.nPos;
    }
};

_IMSTONE_PRIVATE_END
