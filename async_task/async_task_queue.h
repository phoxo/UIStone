#pragma once
#include "async_task.h"

/// The derived class should not use these three messages: WM_APP + 31/32/33.
class CAsyncTaskQueue : public FCMessageWindow
{
private:
    std::map<int, std::shared_ptr<CAsyncTask>>   m_running;
    std::deque<std::shared_ptr<CAsyncTask>>   m_waiting;
    int   m_max_thread_count = 1; // default maximum 1 thread

public:
    struct AddTaskOption
    {
        bool   dispatch_after_add = true;
        bool   add_front = false;
    };

    static constexpr AddTaskOption   AddTaskNoDispatch{ .dispatch_after_add = false };

public:
    CAsyncTaskQueue(PCWSTR window_name = L"")
    {
        CreateMessageWindow(window_name);
    }

    ~CAsyncTaskQueue() override
    {
        BlockWaitAndDiscardAllTask();
        DestroyMessageWindow();
    }

    void SetMaxThreadCount(int max_count)
    {
        m_max_thread_count = max_count;
    }

    // !!! dangerous call, ensure proper checks to prevent deadlock
    void BlockWaitAllRunningTaskFinish()
    {
        _BlockWaitTaskFinish();
        // 模拟发消息，只能保证收到任务完成回调，任务线程里如果有AgentSendMessage可能会丢失，确定这些丢失不影响程序正常运行
        for (auto& [taskid, _] : m_running)
        {
            PostMessage(GetMessageWindow(), CAsyncTask::MSG_ASYNC_TASK_FINISH, taskid, 0);
        }
    }

    void BlockWaitAndDiscardAllTask()
    {
        InvalidateAllRunningTasks();
        _BlockWaitTaskFinish();
        m_running.clear();
        m_waiting.clear();
    }

    const auto& GetRunningTasks() const { return m_running; }
    const auto& GetWaitingTasks() const { return m_waiting; }

    void AddAsyncTask(CAsyncTask* task, const AddTaskOption& option = AddTaskOption())
    {
        if (option.add_front)
            m_waiting.emplace_front(task);
        else
            m_waiting.emplace_back(task);
        if (option.dispatch_after_add)
        {
            DispatchTask();
        }
    }

    void ClearWaitingTasks()
    {
        m_waiting.clear();
    }

    void InvalidateAllRunningTasks() const
    {
        for (auto& [_, task] : m_running) { task->m_is_valid = false; }
    }

    void DispatchTask();
    void PostDispatchTask() const
    {
        ::PostMessage(GetMessageWindow(), CAsyncTask::MSG_POST_DISPATCH_TASK, 0, 0);
    }

protected:
    virtual void OnBeforeExecuteTask(CAsyncTask* task) {}
    virtual void OnExecuteTaskFinish(CAsyncTask* task) {}
    LRESULT MessageWindowProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
    void _BlockWaitTaskFinish()
    {
        if (m_running.size())
        {
            WCHAR   name[128] = {};
            ::GetWindowText(GetMessageWindow(), name, 128);

            DestroyMessageWindow(); // 任务线程里user可能会用AgentSendMessage发消息
            for (auto& [_, task] : m_running)
            {
                task->WaitWorkFinish();
            }
            CreateMessageWindow(name);
        }
    }

    static VOID CALLBACK execute_task_proc(PTP_CALLBACK_INSTANCE, PVOID Context, PTP_WORK)
    {
        AutoComInitializer   auto_init_COM;
        auto   task = (CAsyncTask*)Context;
        task->Execute();

        if (!PostMessage(task->m_host_queue, CAsyncTask::MSG_ASYNC_TASK_FINISH, task->m_id, 0))
        {
            if (::IsWindow(task->m_host_queue))
            {
                assert(false); // message queue is full ?
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
        if (m_running.size() >= m_max_thread_count)
            break;
        if (!m_waiting.size())
            break;

        auto   task = m_waiting.front();
        m_waiting.pop_front();
        task->m_host_queue = GetMessageWindow();
        OnBeforeExecuteTask(task.get());
        m_running[task->m_id] = task;
        task->m_work = CreateThreadpoolWork(execute_task_proc, task.get(), NULL); assert(task->m_work);
        SubmitThreadpoolWork(task->m_work);
    }
}

inline LRESULT CAsyncTaskQueue::MessageWindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == CAsyncTask::MSG_ASYNC_TASK_FINISH)
    {
        auto   iter = m_running.find((int)wParam);
        if (iter != m_running.end())
        {
            auto   task = iter->second;
            task->WaitWorkFinish();
            m_running.erase(iter);

            task->OnTaskFinish();
            OnExecuteTaskFinish(task.get());
        }
        else { assert(false); }

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
