#pragma once

class FCString
{
public:
    template<class T>
    static void SplitTextByToken(const CString& txt, T& split_txt, PCWSTR tokens)
    {
        split_txt.clear();

        int   curr = 0;
        for (;;)
        {
            CString   tmp = txt.Tokenize(tokens, curr);
            if (curr == -1)
                break;
            split_txt.push_back(tmp);
        }
    }

    static int FindNoCase(const std::deque<CString>& vec, PCWSTR search_str)
    {
        int   idx = 0;
        for (auto& iter : vec)
        {
            if (StrCmpI(iter, search_str) == 0)
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
};
