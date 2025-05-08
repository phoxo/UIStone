#pragma once

class FCHelper
{
public:
    template <typename T>
    static bool IsValidIndex(const T& vec, int index)
    {
        return index >= 0 && index < (int)vec.size();
    }

    template<class T>
    static T& LazyCreateSingleton(std::unique_ptr<T>& obj)
    {
        if (!obj)
        {
            obj = std::make_unique<T>();
        }
        return *obj;
    }
};
