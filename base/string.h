#pragma once

class FCString
{
public:
    template<class T>
    static void SplitTextByToken(const CString& txt, T& output, PCWSTR tokens)
    {
        output.clear();

        int   curr = 0;
        for (;;)
        {
            CString   tmp = txt.Tokenize(tokens, curr);
            if (curr == -1)
                break;
            output.push_back(tmp);
        }
    }

    static std::deque<CString> SplitTextByToken(const CString& txt, PCWSTR tokens)
    {
        std::deque<CString>   vec;
        for (int curr = 0;;)
        {
            CString   tmp = txt.Tokenize(tokens, curr);
            if (curr == -1)
                break;
            vec.push_back(tmp);
        }
        return vec;
    }

    static int FindNoCase(const std::deque<CString>& vec, const CString& search_str)
    {
        for (int idx = 0; auto& iter : vec)
        {
            if (_wcsicmp(iter, search_str) == 0)
                return idx;
            idx++;
        }
        return -1;
    }

    static CString Make(const std::wstring_view& sv)
    {
        return CString(sv.data(), (int)sv.size());
    }

    static CString From(int n)
    {
        CString   t;
        t.Format(L"%d", n);
        return t;
    }

    static CString FormatByteSize(LONGLONG bytes)
    {
        WCHAR   buf[32] = {};
        ::StrFormatByteSize(bytes, buf, 32);
        return buf;
    }
};
