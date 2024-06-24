#pragma once
_UISTONE_BEGIN

/// Window helper.
class FCWnd
{
public:
    static CRect GetWindowRect(HWND wnd)
    {
        CRect   t;
        ::GetWindowRect(wnd, t);
        return t;
    }

    static CRect GetClientRect(HWND wnd)
    {
        CRect   t;
        ::GetClientRect(wnd, t);
        return t;
    }

    static int GetScrollbarWidth() { return GetSystemMetrics(SM_CXVSCROLL); }

#ifdef _AFX
    static BOOL IsVScrollVisible(const CWnd& wnd) { return (wnd.GetStyle() & WS_VSCROLL); }

    static int GetScrollbarPos(CWnd& wnd, int code, int bar = SB_VERT)
    {
        SCROLLINFO   si = { 0 };
        wnd.GetScrollInfo(bar, &si);
        return ((code == SB_THUMBPOSITION) || (code == SB_THUMBTRACK)) ? si.nTrackPos : si.nPos;
    }

    static CRect GetChildRectOnParent(const CWnd& parent, int child_ctrl_id)
    {
        CRect   rc;
        ::GetWindowRect(::GetDlgItem(parent, child_ctrl_id), rc);
        parent.ScreenToClient(rc);
        return rc;
    }
#endif

    static void LimitWindowInScreen(CRect& rc)
    {
        CRect   work_rect;
        ::SystemParametersInfo(SPI_GETWORKAREA, sizeof(RECT), work_rect, 0);
        CRect   tmp(rc.TopLeft(), work_rect.Size()); // rc大小限制在屏幕范围内
        rc.IntersectRect(CRect(rc), tmp);

        CSize   delta(0, 0);
        if (rc.left < 0)   delta.cx = -rc.left;
        if (rc.right > work_rect.right)   delta.cx = (work_rect.right - rc.right);
        if (rc.top < 0)   delta.cy = -rc.top;
        if (rc.bottom > work_rect.bottom)   delta.cy = (work_rect.bottom - rc.bottom);
        rc.OffsetRect(delta);
    }
};

_UISTONE_END
