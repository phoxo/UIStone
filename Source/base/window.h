#pragma once
UISTONE_BEGIN

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

        CSize   delta(0, 0);
        if (rc.left < 0)   delta.cx = -rc.left;
        if (rc.right > work_rect.right)   delta.cx = (work_rect.right - rc.right);
        if (rc.top < 0)   delta.cy = -rc.top;
        if (rc.bottom > work_rect.bottom)   delta.cy = (work_rect.bottom - rc.bottom);
        rc.OffsetRect(delta);
    }
};

UISTONE_END
