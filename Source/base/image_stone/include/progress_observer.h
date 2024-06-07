#pragma once

/// Receive progress of process.
class FCProgressObserver
{
private:
    bool   m_canceled = false;
    int   m_fix_progress = -1;

public:
    virtual ~FCProgressObserver() {}

    bool IsUserCanceled() const { return m_canceled; }
    void BeginFixProgress(int fix_progress) { m_fix_progress = fix_progress; }
    void EndFixProgress() { m_fix_progress = -1; }

    bool UpdateProgress(int finish_percent)
    {
        if (m_fix_progress != -1)
            finish_percent = m_fix_progress;

        if (!OnProgressUpdate(finish_percent))
        {
            m_canceled = true;
        }
        return !m_canceled;
    }

protected:
    /// return false if you wanna stop process, return true to continue\n 0 <= finish_percent <= 100
    virtual bool OnProgressUpdate(int finish_percent) = 0;
};
