#pragma once

class CAsyncTask
{
private:
    enum
    {
        MSG_ASYNC_TASK_FINISH = (WM_APP + 31),
        MSG_TASK_SENDMESSAGE_REDIRECT = (WM_APP + 32),
        MSG_POST_DISPATCH_TASK = (WM_APP + 33),
    };

private:
    HWND   m_host_queue = NULL;
    PTP_WORK   m_work = NULL;

public:
    const int   m_id = s_curr_id++;
    bool   m_is_valid = true;

public:
    CAsyncTask() {}

    virtual ~CAsyncTask()
    {
        assert(!m_work);
    }

    void WaitWorkFinish()
    {
        if (m_work)
        {
            WaitForThreadpoolWorkCallbacks(m_work, TRUE);
            CloseThreadpoolWork(m_work);
            m_work = NULL;
        }
    }

    // 1) callback executed from the thread pool
    // 2) 如果需要SendMessage通知主线程，use AgentSendMessage
    virtual void Execute() = 0;

    // 1) callback executed in the MAIN THREAD
    // 2) 如果task被父队列强制丢弃，不会收到此回调
    virtual void OnTaskFinish() {}

    // 禁止从task中调用SendMessage直接发给窗口，CAsyncTaskQueue销毁时可能会阻塞等待任务结束，形成死锁
    // 使用这个代理函数可以安全的给主线程SendMessage
    LRESULT AgentSendMessage(HWND wnd, UINT msg, WPARAM wParam = 0, LPARAM lParam = 0)
    {
        MSG   dest = { wnd, msg, wParam, lParam };
        return ::SendMessage(m_host_queue, MSG_TASK_SENDMESSAGE_REDIRECT, (WPARAM)&dest, 0);
    }

private:
    static inline int   s_curr_id = 1;

    friend class CAsyncTaskQueue;
};
