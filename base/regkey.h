#pragma once

#ifdef _AFX
class FCRegKey : public CRegKey
{
public:
    static CString GetString(HKEY root, PCWSTR key, PCWSTR value)
    {
        CString   out;
        if (DWORD bytes = 0; RegGetValue(root, key, value, RRF_RT_REG_SZ, NULL, NULL, &bytes) == ERROR_SUCCESS)
        {
            auto   hr = RegGetValue(root, key, value, RRF_RT_REG_SZ, NULL, out.GetBuffer(bytes / 2 + 2), &bytes);
            out.ReleaseBuffer();

            // 两次操作非原子，第二次读取只要短于缓冲区就算成功
            // vector 不一样，String遇到\0自动结束，vector读取后要判断是否一样
            if (hr != ERROR_SUCCESS)
                out = L"";
        }
        return out;
    }

    std::vector<int> LoadVector(PCWSTR name)
    {
        std::vector<int>   vec;
        ULONG   bytes = 0;
        if (int count = GetInt32Count(name, bytes))
        {
            vec.resize(count);
            if (ULONG read = bytes; QueryBinaryValue(name, vec.data(), &read) != ERROR_SUCCESS || bytes != read)
                vec.clear();
        }
        return vec;
    }

private:
    int GetInt32Count(PCWSTR name, ULONG& bytes)
    {
        LONG   hr = QueryBinaryValue(name, NULL, &bytes);
        if (hr == ERROR_SUCCESS || hr == ERROR_MORE_DATA)
        {
            int   count = bytes / 4;
            if (count && (bytes % 4 == 0))
                return count;
        }
        return 0;
    }
};
#endif
