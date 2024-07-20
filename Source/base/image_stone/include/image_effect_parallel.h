#pragma once

namespace imagestone_inner
{

/// Parallel process part of an image.
class FCParallelEffectTask
{
private:
    CRect   m_task_region;
    FCImageEffect   & m_effect;
    FCImage   & m_img;
    CHandle   m_finish_event;
    PTP_WORK   m_work = NULL;

public:
    FCParallelEffectTask(const RECT& task_region, FCImageEffect& effect, FCImage& img) : m_task_region(task_region), m_effect(effect), m_img(img) {}

    virtual ~FCParallelEffectTask()
    {
        if (m_work)
        {
            WaitForThreadpoolWorkCallbacks(m_work, FALSE);
            CloseThreadpoolWork(m_work);
        }
    }

    int GetStartPositionPercent() const
    {
        return (100 * m_task_region.top / m_img.Height());
    }

    HANDLE Execute()
    {
        m_finish_event.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
        m_work = CreateThreadpoolWork(execute_task_proc, this, NULL); assert(m_work);
        SubmitThreadpoolWork(m_work);
        return m_finish_event;
    }

private:
    void ProcessInThread()
    {
        auto   type = m_effect.QueryProcessMode();
        if (type == FCImageEffect::ProcessMode::PixelByPixel)
        {
            for (int y = m_task_region.top; y < m_task_region.bottom; y++)
            {
                BYTE   * curr = m_img.GetBits(y);
                for (int x = m_task_region.left; x < m_task_region.right; x++, curr += (m_img.ColorBits() / 8))
                {
                    m_effect.ProcessPixel(m_img, x, y, (RGBA32bit*)curr);
                }
            }
        }
        else if (type == FCImageEffect::ProcessMode::Region)
        {
            m_effect.ProcessRegion(m_img, m_task_region, nullptr);
        }
        ::SetEvent(m_finish_event);
    }

    static VOID CALLBACK execute_task_proc(PTP_CALLBACK_INSTANCE, PVOID Context, PTP_WORK)
    {
        ((FCParallelEffectTask*)Context)->ProcessInThread();
    }
};

class FCParallelEffectTaskExecutor
{
private:
    int   m_max_parallel;
    std::deque<std::shared_ptr<FCParallelEffectTask>>   m_waiting_task;
    std::vector<std::shared_ptr<FCParallelEffectTask>>   m_running_task;
    std::vector<HANDLE>   m_running_finish_event;

public:
    FCParallelEffectTaskExecutor(FCImage& img, FCImageEffect& effect)
    {
        m_max_parallel = effect.QueryMaxParallelCount();
        m_running_task.reserve(m_max_parallel);
        m_running_finish_event.reserve(m_max_parallel);
        SplitParallelTasks(img, effect);
    }

    void ParallelExecuteTask(FCProgressObserver* progress)
    {
        for (;;)
        {
            RunWaitingTasks();
            if (m_running_task.empty())
                break;

            WaitFinishEvent(false); // wait only one task finish
            if (progress && !progress->UpdateProgress(GetCurrentPercent()))
            {
                WaitFinishEvent(true); // wait all finish
                break;
            }
        }
    }

private:
    void SplitParallelTasks(FCImage& img, FCImageEffect& effect)
    {
        CRect   rc(0, 0, img.Width(), img.Height());
        CSize   region = effect.QueryScanLineCountEachParallelTask(img);
        if (region.cx)
            rc.right = region.cx; // split horizontally from left to right
        else
            rc.bottom = region.cy; // split vertically from top to bottom

        for (;;)
        {
            rc.bottom = __min(rc.bottom, img.Height());
            rc.right = __min(rc.right, img.Width());
            m_waiting_task.push_back(std::make_shared<FCParallelEffectTask>(rc, effect, img));

            if ((rc.bottom == img.Height()) && (rc.right == img.Width()))
                break; // <--- the last task
            rc.OffsetRect(region);
        }
    }

    int GetCurrentPercent() const
    {
        // the scan line of first task is the smallest
        return (m_running_task.size() ? m_running_task[0]->GetStartPositionPercent() : 100);
    }

    void RunWaitingTasks()
    {
        int   add = m_max_parallel - (int)m_running_task.size();
        for (int i = 0; i < add; i++)
        {
            RunFirstWaitingTask();
        }
    }

    void RunFirstWaitingTask()
    {
        if (m_waiting_task.size())
        {
            auto   task = m_waiting_task.front();
            m_waiting_task.pop_front();

            HANDLE   finish_event = task->Execute();
            m_running_task.push_back(task);
            m_running_finish_event.push_back(finish_event);
        }
    }

    void WaitFinishEvent(bool wait_all)
    {
        if (m_running_task.empty())
            return;

        DWORD   ret = ::WaitForMultipleObjects((DWORD)m_running_finish_event.size(), m_running_finish_event.data(), wait_all, INFINITE);
        if (wait_all)
        {
            if (ret == WAIT_OBJECT_0)
            {
                m_running_task.clear();
                m_running_finish_event.clear();
                return;
            }
        }
        else
        {
            DWORD   idx = ret - WAIT_OBJECT_0;
            if ((ret >= WAIT_OBJECT_0) && (idx < m_running_task.size()))
            {
                m_running_task.erase(m_running_task.begin() + idx);
                m_running_finish_event.erase(m_running_finish_event.begin() + idx);
                return;
            }
        }
        assert(false);
    }
};

}
