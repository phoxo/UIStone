#pragma once

class FCHelper
{
public:
    static bool IsValidDate(int year, int month, int day)
    {
        return (year > 1970) && (year < 2900) && (month >= 1) && (month <= 12) && (day >= 1) && (day <= 31);
    }

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
