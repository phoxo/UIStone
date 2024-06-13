#pragma once
#include "async_task.h"

_UISTONE_BEGIN

class CAsyncTaskQueue : public FCMessageWindow
{
private:
    std::map<int, std::shared_ptr<CAsyncTask>>   m_running_task;
    std::deque<std::shared_ptr<CAsyncTask>>   m_waiting_task;
    CString   m_queue_wnd_name;
    int   m_max_thread_count = 1; // default maximum 1 thread

public:
    CAsyncTaskQueue(PCWSTR window_name = L"")
    {
        m_queue_wnd_name = window_name;
        CreateMessageWindow(window_name);
    }
    ~CAsyncTaskQueue()
    {
        BlockWaitAndDiscardAllTask();
        DestroyMessageWindow();
    }

    void SetMaxThreadCount(int max_count)
    {
        m_max_thread_count = max_count;
    }

    // !!! dangerous call, check carefully to prevent deadlock
    void BlockWaitAllRunningTaskFinish()
    {
        _BlockWaitTaskFinish();
        // 模拟发消息，只能保证收到任务完成回调，任务线程里如果有AgentSendMessage可能会丢失，确定这些丢失不影响程序正常运行
        for (auto& iter : m_running_task)
        {
            PostMessage(GetMessageWindow(), CAsyncTask::MSG_ASYNC_TASK_FINISH, iter.first, 0);
        }
    }

    void BlockWaitAndDiscardAllTask()
    {
        InvalidateAllRunningTask();
        _BlockWaitTaskFinish();
        m_running_task.clear();
        m_waiting_task.clear();
    }

    const auto& GetRunningTask() const { return m_running_task; }

    void AddAsyncTask(CAsyncTask* task, BOOL dispatch_after_add = TRUE)
    {
        m_waiting_task.push_back(std::shared_ptr<CAsyncTask>(task));
        if (dispatch_after_add)
        {
            DispatchTask();
        }
    }

    void ClearWaitingTask()
    {
        m_waiting_task.clear();
    }

    void InvalidateAllRunningTask() const
    {
        for (auto& iter : m_running_task) { iter.second->m_is_valid = FALSE; }
    }

protected:
    void PostDispatchTask()
    {
        PostMessage(GetMessageWindow(), CAsyncTask::MSG_POST_DISPATCH_TASK, 0, 0);
    }

    virtual void OnBeforeExecuteTask(CAsyncTask* task) {}
    virtual void OnExecuteTaskFinish(CAsyncTask* task) {}
    virtual LRESULT MessageWindowProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    void DispatchTask();

    void _BlockWaitTaskFinish()
    {
        if (m_running_task.size())
        {
            DestroyMessageWindow(); // 任务线程里user可能会用AgentSendMessage发消息
            for (auto& iter : m_running_task)
            {
                iter.second->WaitWorkFinish();
            }
            CreateMessageWindow(m_queue_wnd_name);
        }
    }

    static VOID CALLBACK execute_task_proc(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
    {
        FCAutoInitializeCOM   auto_init_COM;
        auto   task = (CAsyncTask*)Context;
        task->Execute();

        if (!PostMessage(task->m_host_queue, CAsyncTask::MSG_ASYNC_TASK_FINISH, task->m_id, 0))
        {
            if (::IsWindow(task->m_host_queue))
            {
                ASSERT(false); // message queue is full ?
                SendNotifyMessage(task->m_host_queue, CAsyncTask::MSG_ASYNC_TASK_FINISH, task->m_id, 0);
            }
        }
    }
};
//-------------------------------------------------------------------------------------
inline void CAsyncTaskQueue::DispatchTask()
{
    while (true)
    {
        if (m_running_task.size() >= (size_t)m_max_thread_count)
            break;
        if (!m_waiting_task.size())
            break;

        auto   task = m_waiting_task.front();
        m_waiting_task.pop_front();
        if (task->CheckValidBeforeExecute())
        {
            task->m_host_queue = GetMessageWindow();
            OnBeforeExecuteTask(task.get());
            m_running_task[task->m_id] = task;
            task->m_work = CreateThreadpoolWork(execute_task_proc, task.get(), NULL); ASSERT(task->m_work);
            SubmitThreadpoolWork(task->m_work);
        }
    }
}

inline LRESULT CAsyncTaskQueue::MessageWindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == CAsyncTask::MSG_ASYNC_TASK_FINISH)
    {
        auto   iter = m_running_task.find((int)wParam);
        if (iter != m_running_task.end())
        {
            auto   task = iter->second;
            task->WaitWorkFinish();
            m_running_task.erase(iter);

            task->OnTaskFinish();
            OnExecuteTaskFinish(task.get());
        }
        else { ASSERT(false); }

        DispatchTask();
        return 0;
    }
    if (msg == CAsyncTask::MSG_TASK_SENDMESSAGE_REDIRECT)
    {
        MSG   & t = *(MSG*)wParam;
        return ::SendMessage(t.hwnd, t.message, t.wParam, t.lParam);
    }
    if (msg == CAsyncTask::MSG_POST_DISPATCH_TASK)
    {
        DispatchTask();
        return 0;
    }
    return __super::MessageWindowProc(msg, wParam, lParam);
}

_UISTONE_END
