#pragma once

class COwnerDrawButton : public CButton,
                         public ITrackMouseHover
{
private:
    LRESULT   m_check = BST_UNCHECKED;
    CString   m_tip;
    CToolTipCtrl   m_tip_ctrl;

public:
    BOOL Create(const CRect& rc, CWnd* parent_wnd, UINT button_id, DWORD style = WS_VISIBLE)
    {
        return CButton::Create(L"", WS_CHILD | BS_OWNERDRAW | style, rc, parent_wnd, button_id);
    }

    void SetTooltipText(PCWSTR tip) { m_tip = tip; }

protected:
    bool IsButtonHighlight() const { return (GetState() & BST_PUSHED); }

    virtual void OnDrawButton(CDC& dc, CRect client_rect)
    {
    }

    virtual void OnCreateTooltipCtrl(CToolTipCtrl& tip_ctrl)
    {
        m_tip_ctrl.Create(this, TTS_ALWAYSTIP);
    }

    void OnMouse_EnterWnd() override
    {
        if (m_tip.IsEmpty() || m_tip_ctrl)
            return;

        OnCreateTooltipCtrl(m_tip_ctrl);
        m_tip_ctrl.Activate(TRUE);
        m_tip_ctrl.AddTool(this, m_tip, FCWnd::GetClientRect(m_hWnd), 100);
    }

    void PreSubclassWindow() override
    {
        // don't create tooltip window in PreSubclassWindow
        __super::PreSubclassWindow();
        ModifyStyle(0, BS_OWNERDRAW);
    }

    BOOL PreTranslateMessage(MSG* pMsg) override
    {
        if (m_tip_ctrl)
            m_tip_ctrl.RelayEvent(pMsg);
        return __super::PreTranslateMessage(pMsg);
    }

    LRESULT WindowProc(UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        ITrackMouseHover::FilterMouseMessage(m_hWnd, msg);
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

    void DrawItem(LPDRAWITEMSTRUCT lpDIS) override
    {
        CRect   rc;
        GetClientRect(rc);
        CBitmap   bmp;
        bmp.Attach(CreateCompatibleBitmap(lpDIS->hDC, rc.Width(), rc.Height()));
        {
            BitmapHDC   auto_bmp_selected(bmp);
            auto   mem_dc = CDC::FromHandle(auto_bmp_selected);
            SelectObject(*mem_dc, FontManager::GetDefaultFont());
            OnDrawButton(*mem_dc, rc);
            BitBlt(lpDIS->hDC, 0, 0, rc.Width(), rc.Height(), *mem_dc, 0, 0, SRCCOPY);
        }
    }
};
