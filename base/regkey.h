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

            // ���β�����ԭ�ӣ��ڶ��ζ�ȡֻҪ���ڻ���������ɹ�
            // vector ��һ����String����\0�Զ�������vector��ȡ��Ҫ�ж��Ƿ�һ��
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
