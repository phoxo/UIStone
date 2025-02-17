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

    // �е�ʱ�����Ҫ�ŶӺܾã����ֵ�ִ�е�ʱ���ټ��һ�£�������ʱ�򶼲���Ҫִ��������
    virtual bool CheckValidBeforeExecute()
    {
        return m_is_valid;
    }

    // 1) callback in a CHILD THREAD
    // 2) �����ҪSendMessage֪ͨ���̣߳�use AgentSendMessage
    virtual void Execute() = 0;

    // 1) callback in the MAIN THREAD
    // 2) ���task��������ǿ�ƶ����������յ��˻ص�
    virtual void OnTaskFinish() {}

    // ��ֹ��task�е���SendMessageֱ�ӷ������ڣ�CAsyncTaskQueue����ʱ���ܻ������ȴ�����������γ�����
    // ʹ��������������԰�ȫ�ĸ����߳�SendMessage
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
