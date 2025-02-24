#pragma once

//-------------------------------------------------------------------------------------
class ObservedEvent
{
private:
    const int   m_event_type;
public:
    int   m_event_param = 0;
public:
    ObservedEvent(int event_type) : m_event_type(event_type) {}
    virtual ~ObservedEvent() {}
    int GetEventType() const { return m_event_type; }
};
//-------------------------------------------------------------------------------------
// 因为有static inline变量，检查Fire调用时机
// ** 禁止全局/静态类继承
class IEventObserverBase
{
private:
    static inline std::vector<IEventObserverBase*>   g_list;

public:
    IEventObserverBase()
    {
        g_list.push_back(this);
    }

    virtual ~IEventObserverBase()
    {
        std::erase(g_list, this);
    }

    static void FireEvent(int event_type, int event_param = 0)
    {
        ObservedEvent   t(event_type);
        t.m_event_param = event_param;
        FireEvent(t);
    }

    static void FireEvent(ObservedEvent& event)
    {
        for (auto& iter : g_list)
        {
            iter->OnObserveEvent(event);
        }
    }

protected:
    virtual void OnObserveEvent(ObservedEvent& event) = 0;
};
