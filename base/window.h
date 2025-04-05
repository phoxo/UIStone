#pragma once

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
    static CRect GetChildRectOnParent(const CWnd& parent, int child_ctrl_id)
    {
        CRect   rc;
        ::GetWindowRect(::GetDlgItem(parent, child_ctrl_id), rc);
        parent.ScreenToClient(rc);
        return rc;
    }

    static void DDX_Text_NoTip(CDataExchange* pDX, int nIDC, int& t, int tDefault)
    {
        if (pDX->m_bSaveAndValidate)
        {
            CString   s;
            pDX->m_pDlgWnd->GetDlgItemText(nIDC, s);
            t = (s.GetLength() ? StrToInt(s) : tDefault);
        }
        else
        {
            DDX_Text(pDX, nIDC, t);
        }
    }
#endif

    static void LimitWindowInScreen(CRect& rc)
    {
        CRect   work_rect;
        ::SystemParametersInfo(SPI_GETWORKAREA, sizeof(RECT), work_rect, 0);
        CRect   tmp(rc.TopLeft(), work_rect.Size()); // rc大小限制在屏幕范围内
        rc.IntersectRect(CRect(rc), tmp);

        MoveRectInside(rc, work_rect.BottomRight());
    }

    static void MoveRectInside(CRect& rc, const CSize& limit)
    {
        int   dx = OffsetInRange(rc.left, rc.right, limit.cx);
        int   dy = OffsetInRange(rc.top, rc.bottom, limit.cy);
        rc.OffsetRect(dx, dy);
    }

private:
    static int OffsetInRange(int low, int high, int limit)
    {
        if (low < 0)
            return -low;
        else if (high > limit)
            return limit - high;
        return 0;
    }
};
