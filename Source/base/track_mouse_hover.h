#pragma once
UISTONE_BEGIN

/// Monitor mouse enter/leave window.
class FCTrackMouseHover
{
private:
    BOOL   m_track = false;

public:
    virtual ~FCTrackMouseHover() {}

    /// Is mouse pointer is staying on window now.
    BOOL IsMouseHovering() const { return m_track; }

    /// @name Mouse enter/leave window event.
    //@{
    /// Event mouse enter window, default do nothing.
    virtual void OnMouse_EnterWnd() {}
    /// Event mouse leave window, default do nothing.
    virtual void OnMouse_LeaveWnd() {}
    //@}

    /// Derived class must call this function to monitor mouse event.
    void FilterMouseMessage(HWND hWnd, UINT msg)
    {
        if (msg == WM_MOUSEMOVE)
        {
            if (!m_track)
            {
                TRACKMOUSEEVENT   t = { 0 };
                t.cbSize = sizeof(t);
                t.hwndTrack = hWnd;
                t.dwFlags = TME_LEAVE;
                m_track = TrackMouseEvent(&t); assert(m_track);
                OnMouse_EnterWnd();
                ::InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        else if (msg == WM_MOUSELEAVE)
        {
            if (m_track)
            {
                // it's unnecessary to override OnKillFocus,
                // because we can receive WM_MOUSELEAVE when the window deactive.
                m_track = false;
                OnMouse_LeaveWnd();
                ::InvalidateRect(hWnd, NULL, TRUE);
            }
        }
    }
};

UISTONE_END
