#pragma once
#include <set>
_UISTONE_BEGIN

//-------------------------------------------------------------------------------------
class CUIEventParam
{
public:
    virtual ~CUIEventParam() {}
    virtual int GetType() = 0;

protected:
    static int CreateTypeID()
    {
        static int   s_id = 1;
        return s_id++;
    }
};
//-------------------------------------------------------------------------------------
#define DECLARE_EVENT_TYPE_ID \
static int TYPE_ID() \
{ \
    static int   s = CreateTypeID(); \
    return s; \
} \
virtual int GetType() {return TYPE_ID();}
//-------------------------------------------------------------------------------------
class CUIEventObserver
{
private:
    static inline std::set<CUIEventObserver*>   * g_list = NULL;

public:
    CUIEventObserver()
    {
        if (!g_list)
        {
            g_list = new std::set<CUIEventObserver*>();
        }
        g_list->insert(this);
    }

    virtual ~CUIEventObserver()
    {
        g_list->erase(this);
        if (g_list->size() == 0)
        {
            delete g_list;
            g_list = NULL;
        }
    }

    static void FireEvent(CUIEventParam& para)
    {
        if (g_list)
        {
            for (auto iter : (*g_list))
            {
                iter->OnUIEventNotify(para);
            }
        }
    }

protected:
    virtual void OnUIEventNotify(CUIEventParam& para) = 0;
};

_UISTONE_END
