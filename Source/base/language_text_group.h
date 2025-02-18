#pragma once

class LanguageTextGroup
{
private:
    std::deque<CString>   m_text_list;
    const CString   m_empty;

public:
    LanguageTextGroup(PCWSTR section, PCWSTR key = L"t")
    {
        CString   txt = LanguageText::Get(section, key); // key == "t" means total, read all text, then split
        CStringHelper::SplitTextByToken(txt, m_text_list, L"|");
    }

    const CString& operator[](int idx) const
    {
        return FCHelper::IsValidIndex(m_text_list, idx) ? m_text_list[idx] : m_empty;
    }
};
