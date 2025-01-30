#pragma once
#include <optional>

class AutoRestoreHDC
{
private:
    HDC   m_dc;

public:
    AutoRestoreHDC(HDC dc, std::optional<int> select_stock_obj = std::nullopt) : m_dc(dc)
    {
        ::SaveDC(dc);
        if (select_stock_obj)
        {
            SelectObject(dc, GetStockObject(*select_stock_obj));
        }
    }

    ~AutoRestoreHDC()
    {
        ::RestoreDC(m_dc, -1);
    }
};
