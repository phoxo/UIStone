#pragma once
#include "async_task.h"

class CAsyncTaskQueue : public FCMessageWindow
{
private:
    std::map<int, std::shared_ptr<CAsyncTask>>   m_running_task;
    std::deque<std::shared_ptr<CAsyncTask>>   m_waiting_task;
    int   m_max_thread_count = 1; // default maximum 1 thread

public:
    struct AddTaskOption
    {
        bool   dispatch_after_add = true;
        bool   add_front = false;
    };

public:
    CAsyncTaskQueue(PCWSTR window_name = L"")
    {
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

    // !!! dangerous call, ensure proper checks to prevent deadlock
    void BlockWaitAllRunningTaskFinish()
    {
        _BlockWaitTaskFinish();
        // ģ�ⷢ��Ϣ��ֻ�ܱ�֤�յ�������ɻص��������߳��������AgentSendMessage���ܻᶪʧ��ȷ����Щ��ʧ��Ӱ�������������
        for (auto& [taskid, _] : m_running_task)
        {
            PostMessage(GetMessageWindow(), CAsyncTask::MSG_ASYNC_TASK_FINISH, taskid, 0);
        }
    }

    void BlockWaitAndDiscardAllTask()
    {
        InvalidateAllRunningTasks();
        _BlockWaitTaskFinish();
        m_running_task.clear();
        m_waiting_task.clear();
    }

    auto& GetRunningTask() const { return m_running_task; }
    auto& GetWaitingTask() const { return m_waiting_task; }

    void AddAsyncTask(CAsyncTask* task, const AddTaskOption& option = AddTaskOption())
    {
        if (option.add_front)
            m_waiting_task.push_front(std::shared_ptr<CAsyncTask>(task));
        else
            m_waiting_task.push_back(std::shared_ptr<CAsyncTask>(task));
        if (option.dispatch_after_add)
        {
            DispatchTask();
        }
    }

    void ClearWaitingTasks()
    {
        m_waiting_task.clear();
    }

    void InvalidateAllRunningTasks() const
    {
        for (auto& [_, task] : m_running_task) { task->m_is_valid = false; }
    }

protected:
    void PostDispatchTask() const
    {
        ::PostMessage(GetMessageWindow(), CAsyncTask::MSG_POST_DISPATCH_TASK, 0, 0);
    }

    bool IsAllTasksCompleted() const
    {
        return m_running_task.empty() && m_waiting_task.empty();
    }

    virtual void OnBeforeExecuteTask(CAsyncTask* task) {}
    virtual void OnExecuteTaskFinish(CAsyncTask* task) {}
    LRESULT MessageWindowProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
    void DispatchTask();

    void _BlockWaitTaskFinish()
    {
        if (m_running_task.size())
        {
            WCHAR   name[128] = {};
            ::GetWindowText(GetMessageWindow(), name, 128);

            DestroyMessageWindow(); // �����߳���user���ܻ���AgentSendMessage����Ϣ
            for (auto& [_, task] : m_running_task)
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
            task->m_work = CreateThreadpoolWork(execute_task_proc, task.get(), NULL); assert(task->m_work);
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
