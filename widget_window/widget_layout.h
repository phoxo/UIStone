#pragma once
#include "widget_item.h"

using CWidgetItemPtr = std::unique_ptr<CWidgetItem>;

class CWidgetLayout
{
private:
    bool   m_modifying_scrollbar_visible = false;

public:
    virtual ~CWidgetLayout() {}
    virtual void LayoutWidget(CWnd& wnd, std::deque<CWidgetItemPtr>& child_items) = 0;

    bool IsModifyingScrollbar() const { return m_modifying_scrollbar_visible; }

protected:
    /// Set window's vertical scrollbar range.
    void SetVScrollRange(CWnd& wnd, int canvas_height)
    {
        CRect   rc = FCWnd::GetWindowRect(wnd);
        SetScrollbarRange(wnd, canvas_height, rc.Height(), SB_VERT);
    }

    void SetHScrollRange(CWnd& wnd, int canvas_width)
    {
        CRect   rc = FCWnd::GetWindowRect(wnd);
        SetScrollbarRange(wnd, canvas_width, rc.Width(), SB_HORZ);
    }

private:
    void SetScrollbarRange(CWnd& wnd, int canvas_size, int wnd_size, int bar)
    {
        // ��ʾ/���ع�����ShowScrollBar�����send WM_SIZE��һ��layout����ʵ���ӱ���Ҳû���⣬��һ������info�����β���һ����
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
        DWORD   t = wnd.GetStyle(); // Ŀǰ�Ĵ��ڶ�ֻ��ʾһ��H or V������
        return (t & WS_VSCROLL) || (t & WS_HSCROLL);
    }
};
//-------------------------------------------------------------------------------------
class CVerticalWidgetLayout : public CWidgetLayout
{
private:
    int   m_item_height;
    CRect   m_margin;

public:
    CVerticalWidgetLayout(int item_height, CRect margin = CRect()) : m_item_height(item_height), m_margin(margin) {}

private:
    void LayoutWidget(CWnd& wnd, std::deque<CWidgetItemPtr>& child_items) override
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
