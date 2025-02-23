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
    const int   m_id = GenerateID();
    BOOL   m_is_valid = true;

    enum { MSG_USER_BEGIN = (WM_APP + 100) };

public:
    CAsyncTask() {}
    virtual ~CAsyncTask() { assert(!m_work); }

    void WaitWorkFinish()
    {
        if (m_work)
        {
            WaitForThreadpoolWorkCallbacks(m_work, TRUE);
            CloseThreadpoolWork(m_work);
            m_work = NULL;
        }
    }

    // 有的时候可能要排队很久，等轮到执行的时候再检查一下，可能这时候都不需要执行任务了
    virtual bool CheckValidBeforeExecute()
    {
        return m_is_valid;
    }

    // 1) callback in a CHILD THREAD
    // 2) 如果需要SendMessage通知主线程，use AgentSendMessage
    virtual void Execute() = 0;

    // 1) callback in the MAIN THREAD
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
    static int GenerateID()
    {
        static int   last_id = 1;
        return last_id++;
    }

    friend class CAsyncTaskQueue;
};
