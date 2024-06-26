#pragma once
#include <set>
_UISTONE_BEGIN

//-------------------------------------------------------------------------------------
class ObservedEventBase
{
private:
    int   m_event_type;
public:
    ObservedEventBase(int event_type) : m_event_type(event_type) {}
    virtual ~ObservedEventBase() {}
    int GetEventType() const { return m_event_type; }
};
//-------------------------------------------------------------------------------------
class IEventObserverBase
{
private:
    static inline std::set<IEventObserverBase*>   * g_list = nullptr;

public:
    IEventObserverBase()
    {
        if (!g_list)
        {
            g_list = new std::set<IEventObserverBase*>();
        }
        g_list->insert(this);
    }

    virtual ~IEventObserverBase()
    {
        g_list->erase(this);
        if (g_list->size() == 0)
        {
            delete g_list;
            g_list = nullptr;
        }
    }

    static void FireEvent(ObservedEventBase& para)
    {
        if (g_list)
        {
            for (auto iter : (*g_list))
            {
                iter->OnProcessEvent(para);
            }
        }
    }

protected:
    virtual void OnProcessEvent(CUIEventParam& para) = 0;
};

_UISTONE_END
