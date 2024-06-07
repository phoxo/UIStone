#pragma once
_UISTONE_BEGIN

class FCMessageWindow
{
private:
    HWND   m_wnd = nullptr;

public:
    virtual ~FCMessageWindow()
    {
        ASSERT(!m_wnd);
        DestroyMessageWindow();
    }

    HWND GetMessageWindow() const { return m_wnd; }

    void CreateMessageWindow(PCWSTR window_name = NULL)
    {
        ASSERT(!m_wnd);
        DestroyMessageWindow();

        CString   clsname;
        clsname.Format(L"uistone_msgwnd_%I64d", (INT64)&__ImageBase);

        WNDCLASSEX   w = { sizeof(WNDCLASSEX) };
        w.hInstance = (HINSTANCE)&__ImageBase;
        w.lpszClassName = clsname;
        w.lpfnWndProc = message_handle_proc;
        ::RegisterClassEx(&w);

        m_wnd = ::CreateWindowEx(0, w.lpszClassName, window_name, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, w.hInstance, 0);
        ::SetWindowLongPtr(m_wnd, GWLP_USERDATA, (LONG_PTR)this);
    }

    void DestroyMessageWindow()
    {
        if (m_wnd)
        {
            ::SetWindowLongPtr(m_wnd, GWLP_USERDATA, 0);
            ::DestroyWindow(m_wnd);
            ASSERT(!::IsWindow(m_wnd)); // failure to destroy in another thread
        }
        m_wnd = nullptr;
    }

protected:
    virtual LRESULT MessageWindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return ::DefWindowProc(m_wnd, msg, wParam, lParam);
    }

private:
    static LRESULT CALLBACK message_handle_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto   p = (FCMessageWindow*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (p)
        {
            return p->MessageWindowProc(msg, wParam, lParam);
        }
        return ::DefWindowProc(hWnd, msg, wParam, lParam);
    }
};

_UISTONE_END
