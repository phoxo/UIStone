#pragma once
#include "widget_item.h"
UISTONE_BEGIN
typedef  std::unique_ptr<CWidgetItem>  CWidgetItemPtr;

class CWidgetLayout
{
private:
    bool   m_modifying_scrollbar_visible = false;

public:
    virtual ~CWidgetLayout() {}
    virtual void LayoutWidget(CWnd& wnd, std::deque<CWidgetItemPtr>& child_items) = 0;

protected:
    /// Set window's vertical scrollbar range.
    void SetVScrollRange(CWnd& wnd, int canvas_height)
    {
        int   wnd_height = FCWnd::GetWindowRect(wnd).Height();
        SetScrollbarRange(wnd, canvas_height, wnd_height, SB_VERT);
    }

    void SetHScrollRange(CWnd& wnd, int canvas_width)
    {
        int   wnd_width = FCWnd::GetWindowRect(wnd).Width();
        SetScrollbarRange(wnd, canvas_width, wnd_width, SB_HORZ);
    }

private:
    void SetScrollbarRange(CWnd& wnd, int canvas_size, int wnd_size, int bar)
    {
        // 显示/隐藏滚动条ShowScrollBar里面会send WM_SIZE再一次layout，这里返回
        if (m_modifying_scrollbar_visible)
            return;

        m_modifying_scrollbar_visible = true;
        bool   visible = IsAnyScrollbarVisible(wnd);
        if (canvas_size > wnd_size)
        {
            if (!visible)
            {
                wnd.SetScrollPos(bar, 0, FALSE);
                wnd.ShowScrollBar(bar, TRUE);
            }
            CallSetScrollInfo(wnd, canvas_size, wnd_size, bar);
        }
        else
        {
            if (visible)
            {
                wnd.ShowScrollBar(bar, FALSE);
            }
        }
        m_modifying_scrollbar_visible = false;
    }

    static void CallSetScrollInfo(CWnd& wnd, int canvas_size, int wnd_size, int bar)
    {
        SCROLLINFO   si = { .fMask = SIF_RANGE | SIF_PAGE };
        si.nMax = canvas_size - 1; // note -1
        si.nPage = wnd_size;
        wnd.SetScrollInfo(bar, &si, TRUE);
    }

    static bool IsAnyScrollbarVisible(const CWnd& wnd)
    {
        DWORD   t = wnd.GetStyle(); // 目前的窗口都只显示一个H or V滚动条
        return ((t & WS_VSCROLL) || (t & WS_HSCROLL));
    }
};
//-------------------------------------------------------------------------------------
class CVerticalWidgetLayout : public CWidgetLayout
{
private:
    int   m_item_height;
    CRect   m_margin;

public:
    CVerticalWidgetLayout(int item_height, CRect margin = CRect()) : m_item_height(item_height), m_margin(margin) { }

private:
    virtual void LayoutWidget(CWnd& wnd, std::deque<CWidgetItemPtr>& child_items)
    {
        CRect   rc = FCWnd::GetClientRect(wnd);
        rc.DeflateRect(m_margin);
        rc.bottom = rc.top + m_item_height;
        for (auto& iter : child_items)
        {
            iter->SetRectOnCanvas(rc);
            rc.OffsetRect(0, m_item_height);
        }
    }
};

UISTONE_END
