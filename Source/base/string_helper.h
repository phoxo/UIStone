#pragma once
#include <deque>
_UISTONE_BEGIN

class CStringHelper
{
public:
    static void SplitTextByToken(const CString& txt, std::deque<CString>& split_txt, PCWSTR tokens)
    {
        split_txt.clear();

        int   find_pos = 0;
        for (;;)
        {
            CString   tmp = txt.Tokenize(tokens, find_pos);
            if (find_pos == -1)
                break;
            split_txt.push_back(tmp);
        }
    }

    static CString FormatByteSize(LONGLONG bytes)
    {
        TCHAR   buf[32] = { 0 };
        ::StrFormatByteSize(bytes, buf, 32);
        return buf;
    }

    template<class T>
    static CString FormatOneValue(PCTSTR fmt, const T& v)
    {
        CString   k;
        k.Format(fmt, v);
        return k;
    }

    static int FindNoCase(const std::deque<CString>& vec, PCTSTR search_str)
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
};

_UISTONE_END
