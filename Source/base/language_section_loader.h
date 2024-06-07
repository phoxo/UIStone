#pragma once
_UISTONE_BEGIN

class CLanguageSectionLoader
{
private:
    std::deque<CString>   m_text_list;

public:
    CLanguageSectionLoader(PCWSTR section, PCWSTR key = L"t")
    {
        CString   txt = CLanguageText::Get(section, key); // t means total, read all text, then split
        CStringHelper::SplitTextByToken(txt, m_text_list, L"|");
    }

    CString operator[](int idx) const
    {
        if ((idx >= 0) && (idx < m_text_list.size()))
        {
            return m_text_list[idx];
        }
        return L"";
    }
};

_UISTONE_END
