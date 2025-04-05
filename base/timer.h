#pragma once
#include <map>

/// Timer notify.
// ** 因为有static inline变量，禁止全局/静态类继承
class ITimerNotify
{
private:
    UINT_PTR   m_timer_id = 0;

public:
    virtual ~ITimerNotify()
    {
        EndTimer();
    }

    /// Start timer.
    void StartTimer(UINT ms_interval)
    {
        assert(m_timer_id == 0);
        EndTimer();
        m_timer_id = ::SetTimer(NULL, 0, ms_interval, uistone_TimerProc); assert(m_timer_id);
        if (m_timer_id)
        {
            g_timer_list[m_timer_id] = this;
        }
    }

    /// End timer.
    void EndTimer()
    {
        if (m_timer_id)
        {
            g_timer_list.erase(m_timer_id);
            ::KillTimer(NULL, m_timer_id);
        }
        m_timer_id = 0;
    }

protected:
    /// Override to process timer event.
    virtual void OnHandleTimer() = 0;

private:
    static inline std::map<UINT_PTR, ITimerNotify*>   g_timer_list;

    static VOID CALLBACK uistone_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
    {
        // assert(g_timer_list.contains(idEvent)); // 碰到过KillTimer后又进来的，可能是messages already posted to the message queue.

        auto   iter = g_timer_list.find(idEvent);
        if (iter != g_timer_list.end())
            iter->second->OnHandleTimer();
    }
};
