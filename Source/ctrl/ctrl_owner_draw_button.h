#pragma once
_UISTONE_BEGIN

class COwnerDrawButton : public CButton,
                         public FCTrackMouseHover
{
private:
    LRESULT   m_check = BST_UNCHECKED;
    CString   m_tip;
    CToolTipCtrl   m_tip_ctrl;

public:
    BOOL Create(CRect rc, CWnd* parent_wnd, UINT button_id, DWORD style = WS_VISIBLE)
    {
        return CButton::Create(L"", WS_CHILD | BS_OWNERDRAW | style, rc, parent_wnd, button_id);
    }

    void SetTooltipText(PCWSTR tip) { m_tip = tip; }

protected:
    BOOL IsButtonHighlight() const { return (GetState() & BST_PUSHED) ? TRUE : FALSE; }

    virtual void OnDrawButton(CDC& dc, CRect client_rect)
    {
    }

    virtual void OnCreateTooltipCtrl(CToolTipCtrl& tip_ctrl)
    {
        m_tip_ctrl.Create(this, TTS_ALWAYSTIP);
    }

private:
    virtual void OnMouse_EnterWnd()
    {
        if (m_tip.IsEmpty() || m_tip_ctrl)
            return;

        CRect   rc;
        GetClientRect(rc);

        OnCreateTooltipCtrl(m_tip_ctrl);
        m_tip_ctrl.Activate(TRUE);
        m_tip_ctrl.AddTool(this, m_tip, rc, 100);
    }

    virtual void PreSubclassWindow()
    {
        // don't create tooltip window in PreSubclassWindow
        __super::PreSubclassWindow();
        ModifyStyle(0, BS_OWNERDRAW);
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        if (m_tip_ctrl)
            m_tip_ctrl.RelayEvent(pMsg);
        return __super::PreTranslateMessage(pMsg);
    }

    virtual LRESULT WindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        FCTrackMouseHover::FilterMouseMessage(m_hWnd, msg);
        switch (msg)
        {
        case WM_ERASEBKGND: return TRUE;
        case BM_GETCHECK: return m_check;
        case BM_SETCHECK:
            m_check = wParam;
            Invalidate();
            return 0;
        }
        return __super::WindowProc(msg, wParam, lParam);
    }

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS)
    {
        CRect   rc;
        GetClientRect(rc);

        CBitmap   bmp;
        bmp.Attach(CreateCompatibleBitmap(lpDIS->hDC, rc.Width(), rc.Height()));
        {
            FCImageDrawDC   auto_bmp_selected(bmp);
            auto   mem_dc = CDC::FromHandle(auto_bmp_selected);
            SelectObject(*mem_dc, FCFontManager::GetDefaultFont());
            OnDrawButton(*mem_dc, rc);
            BitBlt(lpDIS->hDC, 0, 0, rc.Width(), rc.Height(), *mem_dc, 0, 0, SRCCOPY);
        }
    }
};

_UISTONE_END
