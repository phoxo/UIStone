#pragma once
#include <map>
_UISTONE_BEGIN

//-------------------------------------------------------------------------------------
/**
    Timer notify.
    <span style='color:#FF0000'>don't use it in thread</span>.
*/
class FCTimerNotify
{
    UINT_PTR   m_timer_id = 0;

public:
    FCTimerNotify()
    {
        RegisterTimerObject() ;
    }

    virtual ~FCTimerNotify()
    {
        EndTimer() ;
        UnRegisterTimerObject() ;
    }

    /// Is timer started.
    BOOL IsTimerStarted() const { return (m_timer_id != 0); }

    /**
        Start timer\n
        nInterval - in milliseconds
    */
    void StartTimer (UINT nInterval)
    {
        assert (!IsTimerStarted()) ;
        EndTimer() ;
        m_timer_id = ::SetTimer (NULL, 0, nInterval, FCTimerNotify::uistone_TimerProc) ; assert(m_timer_id);
        if (m_timer_id)
        {
            (*g_timer_list)[m_timer_id] = this ;
        }
    }

    /// End timer.
    void EndTimer()
    {
        if (m_timer_id)
        {
            g_timer_list->erase(m_timer_id) ;
            ::KillTimer (NULL, m_timer_id) ;
        }
        m_timer_id = 0 ;
    }

protected:
    /// Override to process timer event.
    virtual void OnHandleTimer() =0 ;

private:
    typedef std::map<UINT_PTR, FCTimerNotify*> RunningTimerList;

    static inline RunningTimerList* g_timer_list = nullptr; // map of all timer object
    static inline int g_timer_count = 0; // count of timer object

    // object control the life of list to avoid Dead Reference.
    void RegisterTimerObject()
    {
        g_timer_count++;
        if (!g_timer_list)
            g_timer_list = new RunningTimerList;
    }

    void UnRegisterTimerObject()
    {
        g_timer_count--;
        if (g_timer_count == 0)
        {
            delete g_timer_list;
            g_timer_list = nullptr;
        }
    }

    // global timer callback, to dispatch event to timer object.
    static VOID CALLBACK uistone_TimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
    {
        if (!g_timer_list)
            return ;
        auto   iter = g_timer_list->find(idEvent) ;
        if (iter != g_timer_list->end())
            iter->second->OnHandleTimer() ;
    }
};

_UISTONE_END
