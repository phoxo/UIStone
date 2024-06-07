#pragma once
#include "widget_item.h"
_UISTONE_BEGIN
typedef  std::unique_ptr<CWidgetItem>  CWidgetItemPtr;

class CWidgetLayout
{
public:
    virtual ~CWidgetLayout() {}
    virtual void LayoutWidget(CWnd& wnd, std::deque<CWidgetItemPtr>& child_items) = 0;

protected:
    /// Set window's vertical scrollbar range.
    static void SetVScrollRange(CWnd& wnd, int canvas_height)
    {
        BOOL   sb_visible = FCWnd::IsVScrollVisible(wnd);
        int   wnd_height = FCWnd::GetWindowRect(wnd).Height();
        if (canvas_height > wnd_height)
        {
            if (!sb_visible)
            {
                wnd.ShowScrollBar(SB_VERT, TRUE);
            }

            SCROLLINFO   si = { 0, SIF_RANGE | SIF_PAGE };
            si.nMax = canvas_height - 1; // note -1
            si.nPage = wnd_height;
            wnd.SetScrollInfo(SB_VERT, &si, TRUE);
        }
        else
        {
            if (sb_visible)
            {
                wnd.ShowScrollBar(SB_VERT, FALSE);
            }
        }
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

_UISTONE_END
